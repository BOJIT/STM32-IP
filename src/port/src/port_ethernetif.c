#include "port_config.h"

/* lwIP Includes */
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/netifapi.h>
#include <lwip/dhcp.h>
#include <lwip/autoip.h>
#include <lwip/stats.h>
#include <lwip/err.h>
#include <lwip/tcpip.h>

#include <netif/etharp.h>
#include <netif/ethernet.h>

/* Libopencm3 Includes */
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/ethernet/mac.h>
#include <libopencm3/ethernet/phy.h>

/* FreeRTOS Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Phy Register Includes */
#ifdef PHY_LAN8742A
    #include <libopencm3/ethernet/phy_lan87xx.h>
#endif /* PHY_LAN8742A */
#ifdef PHY_KS8081
    #include <libopencm3/ethernet/phy_ksz80x1.h>
#endif /* PHY_KS8081 */

#ifdef DEBUG
#include <stdio.h> // Eventually can be removed, as LWIP handles the debugging
#endif /* DEBUG */

// Network Interface Struct
static struct netif ethernetif;

/* Conversion Struct */
union word_byte {
    u32_t word;
    u8_t byte[4];
};

/*-------------------- Static Global Variables (DMA) -------------------------------*/

/** 
 * @brief Generic DMA Descriptor (see STM32Fxx7 Reference Manual)
*/
struct dma_desc {
    volatile uint32_t   Status;
    uint32_t   ControlBufferSize; // in actuality only first 12 bytes are size
    void *     Buffer1Addr;
    void *     Buffer2NextDescAddr;
    uint32_t   ExtendedStatus;
    uint32_t   Reserved1;
    uint32_t   TimeStampLow;
    uint32_t   TimeStampHigh;
    struct pbuf *pbuf;
};

#ifndef STIF_NUM_TX_DMA_DESC
    #define STIF_NUM_TX_DMA_DESC 10
#endif /* STIF_NUM_TX_DMA_DESC */
static struct dma_desc tx_dma_desc[STIF_NUM_TX_DMA_DESC];
static struct dma_desc *tx_cur_dma_desc;

#ifndef STIF_NUM_RX_DMA_DESC
    #define STIF_NUM_RX_DMA_DESC 10
#endif /* STIF_NUM_RX_DMA_DESC */
static struct dma_desc rx_dma_desc[STIF_NUM_RX_DMA_DESC];
static struct dma_desc *rx_cur_dma_desc;

/* FreeRTOS Semaphore for Ethernet Interrupt */
SemaphoreHandle_t ETH_SEMPHR = NULL;

/*------------------------------ DMA Functions -------------------------------*/

static void init_tx_dma_desc(void)
{
    for (int i = 0; i < STIF_NUM_TX_DMA_DESC; i++) {
        tx_dma_desc[i].Status = ETH_TDES0_TCH | ETH_TDES0_CIC_IPPL;
        tx_dma_desc[i].pbuf = NULL;
        tx_dma_desc[i].Buffer2NextDescAddr = &tx_dma_desc[i+1];
    }
    // Chain buffers in a ring
    tx_dma_desc[STIF_NUM_TX_DMA_DESC-1].Buffer2NextDescAddr = &tx_dma_desc[0];

    ETH_DMATDLAR = (uint32_t) tx_dma_desc; // pointer to start of desc. list
    tx_cur_dma_desc = &tx_dma_desc[0];
}

static void init_rx_dma_desc(void)
{
    for (int i = 0; i < STIF_NUM_RX_DMA_DESC; i++) {
        rx_dma_desc[i].Status = ETH_RDES0_OWN;
        rx_dma_desc[i].ControlBufferSize = ETH_RDES1_RCH | PBUF_POOL_BUFSIZE;
        rx_dma_desc[i].pbuf = pbuf_alloc(PBUF_RAW, PBUF_POOL_BUFSIZE, PBUF_POOL);
        rx_dma_desc[i].Buffer1Addr = rx_dma_desc[i].pbuf->payload;
        rx_dma_desc[i].Buffer2NextDescAddr = &rx_dma_desc[i+1];
    }
    // Chain buffers in a ring
    rx_dma_desc[STIF_NUM_RX_DMA_DESC-1].Buffer2NextDescAddr = &rx_dma_desc[0];
    ETH_DMARDLAR = (uint32_t) rx_dma_desc; // pointer to start of desc. list
    rx_cur_dma_desc = &rx_dma_desc[0];
}

static void prepare_tx_descr(struct pbuf *p, int first, int last)
{
    // wait until the packet is freed by the DMA
    while (tx_cur_dma_desc->Status & ETH_TDES0_OWN);

    // discard old pbuf pointer (in chain)
    if (tx_cur_dma_desc->pbuf != NULL)
        pbuf_free(tx_cur_dma_desc->pbuf);

    // the pbuf field of the descriptor is not used by the DMA
    pbuf_ref(p);
    tx_cur_dma_desc->pbuf = p;

    // tag first and last frames
    tx_cur_dma_desc->Status &= ~(ETH_TDES0_FS | ETH_TDES0_LS);
    if (first)
        tx_cur_dma_desc->Status |= ETH_TDES0_FS;
    if (last)
        tx_cur_dma_desc->Status |= ETH_TDES0_LS;

    tx_cur_dma_desc->Buffer1Addr = p->payload;
    tx_cur_dma_desc->ControlBufferSize = p->len; // overrides default pbuf size
    // ensure that p->len is never greater than 2^13!

    // Pass ownership back to DMA
    tx_cur_dma_desc->Status |= ETH_TDES0_OWN;

    tx_cur_dma_desc = tx_cur_dma_desc->Buffer2NextDescAddr;
}

static int recv_rxdma_buffer(struct netif *netif)
{
    static struct pbuf *first;

    if (rx_cur_dma_desc->Status & ETH_RDES0_OWN)
        return 0;

    if (rx_cur_dma_desc->pbuf == NULL)
        return 1;

    int frame_length = (rx_cur_dma_desc->Status & ETH_RDES0_FL) 
                                                >> ETH_RDES0_FL_SHIFT;

    if (rx_cur_dma_desc->Status & ETH_RDES0_LS)
        frame_length -=4; // Subtract off the CRC

    rx_cur_dma_desc->pbuf->tot_len = frame_length;
    rx_cur_dma_desc->pbuf->len = frame_length;

    if (rx_cur_dma_desc->Status & ETH_RDES0_FS)
        first = rx_cur_dma_desc->pbuf;
    else
        pbuf_cat(first, rx_cur_dma_desc->pbuf);

    if (rx_cur_dma_desc->Status & ETH_RDES0_LS) {
        if (netif->input(first, netif) != ERR_OK)
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
            pbuf_free(first);
        }
    }

    rx_cur_dma_desc->pbuf = NULL;
    rx_cur_dma_desc = rx_cur_dma_desc->Buffer2NextDescAddr;

    return 1;
}

static int realloc_rxdma_buffers(void)
{
    static struct dma_desc *cur_desc = rx_dma_desc;

    if (cur_desc->Status & ETH_RDES0_OWN) {
        cur_desc = rx_cur_dma_desc;
        return 0;
    }

    if (cur_desc->pbuf != NULL)
        return 0;

    cur_desc->pbuf = pbuf_alloc(PBUF_RAW, PBUF_POOL_BUFSIZE, PBUF_POOL);
    if (cur_desc->pbuf == NULL)
        return 1;

    cur_desc->Buffer1Addr = cur_desc->pbuf->payload;
    cur_desc->Status = ETH_RDES0_OWN;

    if (ETH_DMASR & ETH_DMASR_RBUS) {
        ETH_DMASR = ETH_DMASR_RBUS;
        ETH_DMARPDR = 0;
    }

    cur_desc = cur_desc->Buffer2NextDescAddr;

    return 1;
}
// TODO MAKE THESE RX ONE FUNCTION

/*------------------------------- Phy Functions ------------------------------*/

static void phy_negotiate(void)
{
    int regval = ETH_MACCR;
    regval &= ~(ETH_MACCR_DM | ETH_MACCR_FES); // Clear mode and duplex bits

    eth_smi_write(PHY_ADDRESS, PHY_REG_BCR, PHY_REG_BCR_AN); // Autonegotiate

    #ifdef PHY_LAN8742A
        int status;
        while(!((status=eth_smi_read(PHY_ADDRESS, LAN87XX_SCSR)) 
                & LAN87XX_SCSR_AUTODONE));  // Wait for autonegotiation to finish
        switch(status & LAN87XX_SCSR_SPEED) {
            case LAN87XX_SCSR_SPEED_10HD:
                break;
            case LAN87XX_SCSR_SPEED_100HD:
                regval |= ETH_MACCR_FES;
                break;
            case LAN87XX_SCSR_SPEED_10FD:
                regval |= ETH_MACCR_DM;
                break;
            case LAN87XX_SCSR_SPEED_100FD:
                regval |= ETH_MACCR_FES;
                regval |= ETH_MACCR_DM;
                break;
        }
    #endif /* PHY_LAN8742A */
    #ifdef PHY_KS8081
        #error "This PHY is not implemented yet!"
    #endif /* PHY_KS8081 */

    LWIP_DEBUGF(NETIF_DEBUG, ("mac_init: autonegotiate status: %d\n", regval));

    ETH_MACCR = regval;     // Write speed and duplex to control register
}

/*------------------------------- FreeRTOS Tasks -----------------------------*/

/** 
 * @brief Task for handling incoming ethernet packets (triggered by ISR)
 * @param argument = <i>netif</i> struct from lwIP
*/
void ethernetif_input(void* argument)
{
    struct netif *netif = (struct netif *) argument;
    
    for(;;) {

        // This can be done more efficiently with direct-to-task notification
        if(xSemaphoreTake(ETH_SEMPHR, portMAX_DELAY) == pdTRUE) {
            //LOCK_TCPIP_CORE();  // Don't know if locking is really necessary
            int ret = recv_rxdma_buffer(netif);
            ret |= realloc_rxdma_buffers(); // return can be used later
            //UNLOCK_TCPIP_CORE();
        }
    }
}

/** 
 * @brief Polls the phy every 0.5 seconds to check the status
 * @param argument = <i>netif</i> struct from lwIP
 * The phy hardware interrupt is not available if the phy is providing the
 * reference clock, so a simple poll is used.
*/
void ethernetif_phy(void* argument)
{
    struct netif *netif = (struct netif *) argument;
    bool link_status = netif_is_link_up(netif);

    phy_reset(PHY_ADDRESS);

    for(;;) {
        if(link_status != phy_link_isup(PHY_ADDRESS)) {
            link_status = !link_status;
            if(link_status == true) {
                netifapi_netif_set_link_up(netif);      // Link up
            }
            else {
                netifapi_netif_set_link_down(netif);    // Link down
            }
        }
        vTaskDelay(500);
    }
}

/*--------------------------- Operation Functions ----------------------------*/

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    struct pbuf *q;

    // Iterate through pbuf chain until next-> == NULL
    for(q = p; q != NULL; q = q->next)
        prepare_tx_descr(q, q == p, q->next == NULL);

    // check if DMA is waiting for a descriptor it owns
    if (ETH_DMASR & ETH_DMASR_TBUS) {
        ETH_DMASR = ETH_DMASR_TBUS; // acknowledge
        ETH_DMATPDR = 0; // ask DMA to carry on polling
    }
    // the step above is not required if the DMA hasn't got through all the
    // previous descriptors before the next packet is sent. potentially this
    // behaviour should be flagged, as currently nothing is stopping descriptor
    // overflow.

    return ERR_OK;
}

static err_t mac_init(void)
{
    /* Soft reset ethernet MAC */
    ETH_DMABMR |= ETH_DMABMR_SR;
    while (ETH_DMABMR & ETH_DMABMR_SR);

    /* Initialize PHY Communication */
    ETH_MACMIIAR = ETH_CLK_060_100MHZ; // Change depending on HCLK speed

    /* Configure ethernet MAC */
    ETH_MACCR |= (ETH_MACCR_FES | ETH_MACCR_ROD |
                   ETH_MACCR_IPCO | ETH_MACCR_DM);

    ETH_MACFFR |= ETH_MACFFR_RA; // No Filtering Currently
    ETH_MACFCR = 0;

    ETH_DMABMR = (ETH_DMABMR_AAB | ETH_DMABMR_FB | ETH_DMABMR_PM_2_1 |
                   (32 << ETH_DMABMR_RDP_SHIFT) |  //RX Burst Length
                   (32 << ETH_DMABMR_PBL_SHIFT)  |  //TX Burst Length
                   ETH_DMABMR_USP | ETH_DMABMR_EDFE);

    ETH_DMAOMR = (ETH_DMAOMR_DTCEFD | ETH_DMAOMR_RSF |
                   ETH_DMAOMR_TSF | ETH_DMAOMR_OSF);

    return ERR_OK;
}

static void low_level_init(struct netif *netif)
{
    /* set MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    #ifdef MAC_ADDR_MANUAL
        netif->hwaddr[0] = MAC_ADDR_0;
        netif->hwaddr[1] = MAC_ADDR_1;
        netif->hwaddr[2] = MAC_ADDR_2;
        netif->hwaddr[3] = MAC_ADDR_3;
        netif->hwaddr[4] = MAC_ADDR_4;
        netif->hwaddr[5] = MAC_ADDR_5;
    #else
        uint32_t signature[3];
        desig_get_unique_id(signature); // Pull ID from ST's unique ID registers

        union word_byte id_addr;
        id_addr.word = signature[2];    // Use LSB of the Unique ID

        netif->hwaddr[0] = 0x00;       // ST's MAC Prefix
        netif->hwaddr[1] = 0x80;
        netif->hwaddr[2] = 0xE1;
        netif->hwaddr[3] = id_addr.byte[1];
        netif->hwaddr[4] = id_addr.byte[2];
        netif->hwaddr[5] = id_addr.byte[3];
    #endif /* MAC_ADDRESS_MANUAL */

    /* Set MAC address in ethernet peripheral */
    eth_set_mac(netif->hwaddr);

    /* Netif flags */
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    // #if LWIP_IGMP
    // netif->flags |= NETIF_FLAG_IGMP;
    // netif_set_igmp_mac_filter(netif, mac_filter); // LATER
    // #endif

    init_tx_dma_desc();
    init_rx_dma_desc();

    /* FreeRTOS Setup */
    ETH_SEMPHR = xSemaphoreCreateBinary();

    xTaskCreate(ethernetif_input, "ETH_input", 1024, netif, 
                configMAX_PRIORITIES-1, NULL);
    xTaskCreate(ethernetif_phy, "ETH_phy", 350, netif, 1, NULL);

    /* Enable MAC and DMA transmission and reception */
    eth_start(); 

    // Configure interrupts on reception only (PTP may change this later)
    ETH_DMAIER = ETH_DMAIER_RIE | ETH_DMAIER_RBUIE | ETH_DMAIER_NISE;

    /* NVIC Interrupt Configuration */
    nvic_set_priority(NVIC_ETH_IRQ, 5);
    nvic_enable_irq(NVIC_ETH_IRQ);
}

/* Initialise hardware ready for Ethernet */
void eth_hw_init(void)
{
    /* Enable relavant clocks */
    rcc_periph_clock_enable(RCC_ETHMAC);
    rcc_periph_clock_enable(RCC_ETHMACRX);
    rcc_periph_clock_enable(RCC_ETHMACTX);

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOG);

    /* TODO Have not initialised PPS pin for debug! */
    

    /* Configure ethernet GPIOs */
    /* GPIOA */
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP,
            GPIO_OSPEED_50MHZ, GPIO_ETH_RMII_MDIO);
    gpio_set_af(GPIOA, GPIO_AF11, GPIO_ETH_RMII_MDIO |
            GPIO_ETH_RMII_REF_CLK | GPIO_ETH_RMII_CRS_DV);
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_ETH_RMII_MDIO |
            GPIO_ETH_RMII_REF_CLK | GPIO_ETH_RMII_CRS_DV);

    /* GPIOB */
    gpio_set_output_options(GPIOB, GPIO_OTYPE_PP,
            GPIO_OSPEED_50MHZ, GPIO_ETH_RMII_TXD1);
    gpio_set_af(GPIOB, GPIO_AF11, GPIO_ETH_RMII_TXD1);
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_ETH_RMII_TXD1);
    // PPS definition to go here when implemented

    /* GPIOC */
    gpio_set_output_options(GPIOC, GPIO_OTYPE_PP,
            GPIO_OSPEED_50MHZ, GPIO_ETH_RMII_MDC);
    gpio_set_af(GPIOC, GPIO_AF11, GPIO_ETH_RMII_MDC |
            GPIO_ETH_RMII_RXD0 | GPIO_ETH_RMII_RXD1);
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_ETH_RMII_MDC |
            GPIO_ETH_RMII_RXD0 | GPIO_ETH_RMII_RXD1);

    /* GPIOG */
    gpio_set_output_options(GPIOG, GPIO_OTYPE_PP,
            GPIO_OSPEED_50MHZ, GPIO_ETH_RMII_TX_EN | GPIO_ETH_RMII_TXD0);
    gpio_set_af(GPIOG, GPIO_AF11, GPIO_ETH_RMII_TX_EN | GPIO_ETH_RMII_TXD0);
    gpio_mode_setup(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE,
            GPIO_ETH_RMII_TX_EN | GPIO_ETH_RMII_TXD0);

    /* Configure to use RMII interface */
    rcc_periph_clock_enable(RCC_SYSCFG);

    rcc_periph_reset_hold(RST_ETHMAC);
    SYSCFG_PMC |= (1 << 23);    // MII_RMII_SEL (use RMII)
    rcc_periph_reset_release(RST_ETHMAC);

    vTaskDelay(10); // Wait for peripheral to power on
}

/*---------------------------- CALLBACK FUNCTIONS ----------------------------*/

static void ethernetif_status(struct netif *netif)
{
    if(netif_is_up(netif)) {
        #if(LWIP_DHCP==1)
            netifapi_dhcp_start(&ethernetif);
        #endif
        //igmp_start(&netif); // LATER
    }
    else {
        #if(LWIP_DHCP==1)
            netifapi_dhcp_stop(&ethernetif);
        #endif
    }
}

static void ethernetif_link(struct netif *netif)
{
    // Restart services with a semaphore! + any ETH Mac reinitialisation

    if(netif_is_link_up(netif)) {
        phy_negotiate();                // Blocks until negotiation is finished
        netifapi_netif_set_up(netif);
    }
    else {
        netifapi_netif_set_down(netif);
    }
}

err_t ethernetif_init(struct netif *netif)
{
    err_t ret;

    LWIP_ASSERT("netif != NULL", (netif != NULL));

    // #if LWIP_NETIF_HOSTNAME
    //     /* Initialize interface hostname */
    //     netif->hostname = "lwip";
    // #endif /* LWIP_NETIF_HOSTNAME */

    netif->name[0] = 's';
    netif->name[1] = 't';
    netif->output = etharp_output;
    netif->linkoutput = low_level_output;

    if ((ret = mac_init()) != ERR_OK)
        return ret;

    /* initialize the hardware */
    low_level_init(netif);

    return ERR_OK;
}

/*--------------------- PUBLIC DEVICE-SPECIFIC FUNCTIONS ---------------------*/

void networkInit(void)
{
    eth_hw_init();  // Initialise RCC, GPIOs, Registers, etc... (MSP)
    
    tcpip_init(NULL, NULL);     // Initialises main lwIP core thread

    ip_addr_t ip_addr = {0};
    ip_addr_t net_mask = {0};
    ip_addr_t gw_addr = {0};
    #if(LWIP_DHCP==0)           // Static IP configuration
        IP4_ADDR(&ip_addr, 192, 168, 50, 51);
        IP4_ADDR(&net_mask, 255, 255, 255, 0);
        IP4_ADDR(&gw_addr, 192, 168, 50, 1);  
    #endif

    netif_add(&ethernetif, &ip_addr, &net_mask, &gw_addr, NULL,
                       ethernetif_init, tcpip_input); 
    netif_set_default(&ethernetif);

    // netif_set_hostname(&netif, "lwip");

    netif_set_status_callback(&ethernetif, ethernetif_status);
    netif_set_link_callback(&ethernetif, ethernetif_link);

    netifapi_netif_set_link_up(&ethernetif);

    LWIP_DEBUGF(NETIF_DEBUG, ("Mac Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
           ethernetif.hwaddr[0], ethernetif.hwaddr[1], ethernetif.hwaddr[2],
           ethernetif.hwaddr[3], ethernetif.hwaddr[4], ethernetif.hwaddr[5]));
}

void gdb_break() {
    // empty - convenient tag for GDB
    printf("ERROR\n");
    for(;;);
}


/*------------------------------- ETHERNET ISR -------------------------------*/

/* Ethernet ISR */
void eth_isr(void)
{
    static BaseType_t task_yield = pdFALSE;
    // Clear ethernetif_input semaphore
    if((ETH_DMASR & ETH_DMASR_RS) == ETH_DMASR_RS) {    // Packet Recieved
        xSemaphoreGiveFromISR(ETH_SEMPHR, &task_yield);
        portYIELD_FROM_ISR(task_yield);
        ETH_DMASR = ETH_DMASR_RS;
    }
    else {  // Error Condition
        gdb_break();    // TODO properly
    }
    ETH_DMASR = ETH_DMAIER_NISE; // Clear Normal Interrupt Summary
}
