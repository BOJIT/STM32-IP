#include "port_config.h"
#include "port_ethernetif.h"

/* lwIP Includes */
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/dhcp.h>
#include <lwip/autoip.h>
#include <lwip/stats.h>
#include <lwip/err.h>
#include <lwip/netif.h>

#include <netif/etharp.h>

/* Libopencm3 Includes */
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/ethernet/mac.h>
#include <libopencm3/ethernet/phy.h>

#ifdef DEBUG
#include <stdio.h>
#endif /* DEBUG */

// Network Interface Struct
static struct netif netif;

// static enum {
//     NO_CHANGE,
//     LINK_UP,
//     LINK_DOWN,
// } phy_link_status; // Implement later

// static void netif_status(struct netif *n)
// {
//     if (n->flags & NETIF_FLAG_UP) {
//         printf("Interface Up %s:\n",
//                n->flags & NETIF_FLAG_DHCP ? "(DHCP)" : "(STATIC)");

//         printf("  IP Address: " IP_F "\n", IP_ARGS(&n->ip_addr));
//         printf("  Net Mask:   " IP_F "\n", IP_ARGS(&n->netmask));
//         printf("  Gateway:    " IP_F "\n", IP_ARGS(&n->gw));

//         const char *speed = "10Mb/s";
//         if (ETH->MACCR & ETH_MACCR_FES)
//             speed = "100Mb/s";

//         const char *duplex = "Half";
//         if (ETH->MACCR & ETH_MACCR_DM)
//             duplex = "Full";

//         printf("  Mode:       %s  %s Duplex\n", speed, duplex);

//     } else {
//         printf("Interface Down.\n");
//     }
// }

// static void netif_link(struct netif *n)
// {
//     static int dhcp_started = 0;

//     if (n->flags & NETIF_FLAG_LINK_UP) {
//         printf("Link Up.\n");

//         if (!dhcp_started) {
//             dhcp_started = 1;
//             dhcp_start(n);
//         }

//     } else {
//         printf("Link Down.\n");
//     }
// }
/*-------------------- Static Global Variables (DMA) -------------------------------*/
/* from lsgunth */
struct dma_desc {
    volatile uint32_t   Status;
    uint32_t   ControlBufferSize;
    void *     Buffer1Addr;
    void *     Buffer2NextDescAddr;
    uint32_t   ExtendedStatus;
    uint32_t   Reserved1;
    uint32_t   TimeStampLow;
    uint32_t   TimeStampHigh;
    struct pbuf *pbuf;
};

#ifndef STIF_NUM_TX_DMA_DESC
#define STIF_NUM_TX_DMA_DESC 20
#endif
static struct dma_desc tx_dma_desc[STIF_NUM_TX_DMA_DESC];
static struct dma_desc *tx_cur_dma_desc;

#ifndef STIF_NUM_RX_DMA_DESC
#define STIF_NUM_RX_DMA_DESC 5
#endif
static struct dma_desc rx_dma_desc[STIF_NUM_RX_DMA_DESC];
static struct dma_desc *rx_cur_dma_desc;
// These variables are global to avoid silly stack sizes - they should only
// ever be accessed by the Ethernet handler thread!

/*--------------------- PRIVATE DEVICE-SPECIFIC FUNCTIONS --------------------*/
static void init_tx_dma_desc(void)
{
    for (int i = 0; i < STIF_NUM_TX_DMA_DESC; i++) {
        tx_dma_desc[i].Status = ETH_TDES0_TCH | ETH_TDES0_CIC_IPPL;
        tx_dma_desc[i].pbuf = NULL;
        tx_dma_desc[i].Buffer2NextDescAddr = &tx_dma_desc[i+1];
    }
    // Chain buffers in a ring
    tx_dma_desc[STIF_NUM_TX_DMA_DESC-1].Buffer2NextDescAddr = &tx_dma_desc[0];

    ETH_DMATDLAR = (uint32_t) tx_dma_desc;
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

    rx_dma_desc[STIF_NUM_RX_DMA_DESC-1].Buffer2NextDescAddr = &rx_dma_desc[0];
    ETH_DMARDLAR = (uint32_t) rx_dma_desc;
    rx_cur_dma_desc = &rx_dma_desc[0];
}

static err_t mac_init(void)
{
    /* Soft reset ethernet MAC */
    ETH_DMABMR |= ETH_DMABMR_SR;
    while (ETH_DMABMR & ETH_DMABMR_SR);

    /* Initialize PHY Communication */
    ETH_MACMIIAR = ETH_CLK_060_100MHZ; // Change depending on HCLK speed
	phy_reset(PHY_ADDRESS);
    // Check link status initially here

    /* Configure ethernet MAC */
    ETH_MACCR |= (ETH_MACCR_FES | ETH_MACCR_ROD |
                   ETH_MACCR_IPCO | ETH_MACCR_DM);

    ETH_MACFFR = 0;
    ETH_MACFCR = 0;

    ETH_DMAOMR = (ETH_DMAOMR_DTCEFD | ETH_DMAOMR_RSF |
                   ETH_DMAOMR_TSF | ETH_DMAOMR_OSF);

    ETH_DMABMR = (ETH_DMABMR_AAB | ETH_DMABMR_FB | ETH_DMABMR_PM_2_1 |
                   (32 << ETH_DMABMR_RDP_SHIFT) |  //RX Burst Length
                   (32 << ETH_DMABMR_PBL_SHIFT)  |  //TX Burst Length
                   ETH_DMABMR_USP | ETH_DMABMR_EDFE);

    return ERR_OK;
}

err_t netif_init(struct netif *netif)
{
    err_t ret;

    LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

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

/* Initialise hardware ready for Ethernet */
void eth_hw_init(void) {
    /* Enable relavant clocks */
    rcc_periph_clock_enable(RCC_ETHMAC);
    rcc_periph_clock_enable(RCC_ETHMACRX);
    rcc_periph_clock_enable(RCC_ETHMACTX);

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOG);
    

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

    // 2 Clock cycles are required before accessing a peripheral after an RCC change
    asm("NOP"); asm("NOP");
}

/*-------------------------- LOW-LEVEL IO FUNCTIONS --------------------------*/
static void low_level_init(struct netif *netif)
{
    /* set MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    //read_hwaddr_from_otp(netif); // TEMPORARY HARD-CODED MAC ADDRESS
    netif->hwaddr[0] = 0x01;
    netif->hwaddr[1] = 0x02;
    netif->hwaddr[2] = 0x03;
    netif->hwaddr[3] = 0x04;
    netif->hwaddr[4] = 0x05;
    netif->hwaddr[5] = 0x06;

    /* Set MAC Address */
    eth_set_mac(netif->hwaddr);

    /* maximum transfer unit */
    netif->mtu = 1500;

    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    // #if LWIP_IGMP
    // netif->flags |= NETIF_FLAG_IGMP;
    // netif_set_igmp_mac_filter(netif, mac_filter); // LATER
    // #endif

    init_tx_dma_desc();
    init_rx_dma_desc();

    /* Enable MAC and DMA transmission and reception */
    eth_start();

    // ethIRQenable() in libopencm3
    ETH_DMAIER = ETH_DMAIER_ERIE | ETH_DMAIER_RIE | ETH_DMAIER_NISE;

    /* NVIC Interrupt Configuration */
    nvic_set_priority(NVIC_ETH_IRQ, 5);
    nvic_enable_irq(NVIC_ETH_IRQ);

}



/*--------------------- PUBLIC DEVICE-SPECIFIC FUNCTIONS ---------------------*/

void networkInit(void)
{
    eth_hw_init();  // Initialise RCC, GPIOs, Registers, etc... (MSP)

    ip_addr_t ip_addr = {0};
    ip_addr_t net_mask = {0};
    ip_addr_t gw_addr = {0};
 
    lwip_init();
    netif_add(&netif, &ip_addr, &net_mask, &gw_addr, NULL, netif_init,
              ethernet_input);
    //netif_set_status_callback(&netif, netif_status); add these later
    //netif_set_link_callback(&netif, netif_link);
    netif_set_default(&netif);
    netif_set_hostname(&netif, "lwip");

    printf("Mac Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
           netif.hwaddr[0], netif.hwaddr[1], netif.hwaddr[2],
           netif.hwaddr[3], netif.hwaddr[4], netif.hwaddr[5]);

    //igmp_start(&netif); // LATER
}


/*------------------------------- ETHERNET ISR -------------------------------*/

/* Ethernet ISR */
void eth_isr(void) {
    //eth_irq_ack_pending(ETH_DMASR_NIS);
    // Temporarily just clear the interrupt
    ETH_DMASR |= ETH_DMASR_ERS | ETH_DMASR_RS | ETH_DMASR_NIS;
}
