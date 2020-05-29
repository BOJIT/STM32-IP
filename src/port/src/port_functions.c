#include "port_functions.h"

/* Inclue FreeRTOS Headers */
#include "FreeRTOS.h"
#include "task.h"

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

/* LWIP includes */
// #include "lwip/mem.h"
// #include "netif/etharp.h"

/* Global Port-Specific Definitions */
#include "port_config.h"
#include "port_ethernetif.h"

#ifdef DEBUG
#include <stdio.h>
#endif /* DEBUG */

// /*-------------------- Static Global Variables (DMA) -------------------------------*/
// /* from lsgunth */
// struct dma_desc {
//     volatile uint32_t   Status;
//     uint32_t   ControlBufferSize;
//     void *     Buffer1Addr;
//     void *     Buffer2NextDescAddr;
//     uint32_t   ExtendedStatus;
//     uint32_t   Reserved1;
//     uint32_t   TimeStampLow;
//     uint32_t   TimeStampHigh;
//     struct pbuf *pbuf;
// };

// #ifndef STIF_NUM_TX_DMA_DESC
// #define STIF_NUM_TX_DMA_DESC 20
// #endif
// static struct dma_desc tx_dma_desc[STIF_NUM_TX_DMA_DESC];
// static struct dma_desc *tx_cur_dma_desc;

// #ifndef STIF_NUM_RX_DMA_DESC
// #define STIF_NUM_RX_DMA_DESC 5
// #endif
// static struct dma_desc rx_dma_desc[STIF_NUM_RX_DMA_DESC];
// static struct dma_desc *rx_cur_dma_desc;
// // These variables are global to avoid silly stack sizes - they should only
// // ever be accessed by the Ethernet handler thread!

/*---------------------------- Ethernet Driver -------------------------------*/


/*----------------------------------------------------------------------------*/

#ifdef DEBUG
/* Function to Map SYSCLK/4 to GPIO PC9 */
static void prvMcoSetup(void)
{
    /* PA8 to AF 0 for MCO */
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
    gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO9);
    gpio_set_af(GPIOC, 0, GPIO9);

    /* clock output on pin PC9 (allows checking with scope) */
    RCC_CFGR = (RCC_CFGR & ~(RCC_CFGR_MCO2_MASK << RCC_CFGR_MCO2_SHIFT)) |
            (RCC_CFGR_MCO2_SYSCLK << RCC_CFGR_MCO2_SHIFT);

    /* Set clock prescaler */
    RCC_CFGR = (RCC_CFGR & ~(RCC_CFGR_MCOPRE_MASK << RCC_CFGR_MCO2PRE_SHIFT)) |
            (RCC_CFGR_MCOPRE_DIV_4 << RCC_CFGR_MCO2PRE_SHIFT);
}
#endif /* DEBUG */

/*--------------------- PUBLIC DEVICE-SPECIFIC FUNCTIONS ---------------------*/

/* Function to Initialise all GPIOs */
void vLEDInitialize() {
    /* Enable GPIO clocks */
    rcc_periph_clock_enable(SYSTEM_LED_RCC);
    rcc_periph_clock_enable(STATUS_LED_RCC);
    rcc_periph_clock_enable(WARNING_LED_RCC);
    /* System LED */
    gpio_mode_setup(SYSTEM_LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
            SYSTEM_LED_PIN);
    /* Status LED */
    gpio_mode_setup(STATUS_LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
            STATUS_LED_PIN);
    /* Warning LED */
    gpio_mode_setup(WARNING_LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
            WARNING_LED_PIN);
}

/* Function to Toggle System LED */
void vSystemLEDToggle() {
    gpio_toggle(SYSTEM_LED_PORT, SYSTEM_LED_PIN);
}

/* Function to Toggle Status LED */
void vStatusLEDToggle() {
    gpio_toggle(STATUS_LED_PORT, STATUS_LED_PIN);
}

/* Function to Toggle Warning LED */
void vWarningLEDToggle() {
    gpio_toggle(WARNING_LED_PORT, WARNING_LED_PIN);
}

/* Initialise All System Clock Architecture */
// TODO: investigate systick divisor (currently 64 seems to work)
void vConfigureClock() {

    struct rcc_clock_scale rcc_config;

    /* Clock Configuration Settings */
    rcc_config.plln = SYSCLK_FREQ/1000000;
    rcc_config.pllp = 0x02; // PLLP divisor of 2
    rcc_config.pllq = 0x04; // PLLQ divisor of 4
    rcc_config.flash_waitstates = FLASH_ACR_LATENCY_3WS;
    rcc_config.hpre = RCC_CFGR_HPRE_DIV_NONE;
    rcc_config.ppre1 = RCC_CFGR_PPRE_DIV_2;
    rcc_config.ppre2 = RCC_CFGR_PPRE_DIV_NONE;
    rcc_config.vos_scale = PWR_SCALE1; // Max power mode
    rcc_config.overdrive = 0; // No overdrive
    rcc_config.ahb_frequency = SYSCLK_FREQ;
    rcc_config.apb1_frequency = SYSCLK_FREQ/2;
    rcc_config.apb2_frequency = SYSCLK_FREQ;

    /* Write Clock Configuration to RCC */
    rcc_clock_setup_hse(&rcc_config, HSE_FREQ/2000000);

    #ifdef DEBUG
    /* Map system clock to PC9 (MCO2 output) */
    prvMcoSetup();
    #endif /* DEBUG */
}

#ifdef DEBUG
/* Configure UART for debugging messages */
void vConfigureUART() {
    /* Enable GPIOD and USART3 clock. */
    rcc_periph_clock_enable(DEBUG_UART_RCC);
    rcc_periph_clock_enable(RCC_USART3);
    /* Setup GPIO pins for USART3 transmit. */
    gpio_mode_setup(DEBUG_UART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE,
            DEBUG_UART_TX | DEBUG_UART_RX);
    gpio_set_af(DEBUG_UART_PORT, GPIO_AF7, DEBUG_UART_TX | DEBUG_UART_RX);

    /* Setup USART3 parameters. */
    usart_set_baudrate(USART3, 115200);
    usart_set_databits(USART3, 8);
    usart_set_stopbits(USART3, USART_STOPBITS_1);
    usart_set_mode(USART3, USART_MODE_TX);
    usart_set_parity(USART3, USART_PARITY_NONE);
    usart_set_flow_control(USART3, USART_FLOWCONTROL_NONE);

    /* Finally enable the USART. */
    usart_enable(USART3);
}
#endif /* DEBUG */

/* TEMPORARY GLOBAL VARIABLES! --------------------------------------------------- */
// Eventually the ethernet descriptors will be declared in the task that
// initialises the ethernet peripheral and handles the ethernet callbacks


// uint8_t en_mac[6] = {1, 2, 3, 4, 5, 6}; // For example purposes only

// struct eth_pkt {
//     uint8_t dest_mac[6];
//     uint8_t src_mac[6];
//     uint16_t ethertype;
//     uint8_t data[100];
// } pkt;

// union eth_frame_type {
//     struct eth_pkt pkt;
//     uint8_t data[sizeof(pkt)/sizeof(uint8_t)];
// } eth_frame;

/* TEMPORARY GLOBAL VARIABLES! --------------------------------------------------- */

/* Configure Ethernet Peripheral */
void vConfigureETH() {

    networkInit();
    // /* Soft MAC Reset */
    // ETH_DMABMR |= ETH_DMABMR_SR;
    // while (ETH_DMABMR & ETH_DMABMR_SR); 

    // /* Initialise Ethernet Hardware */
    // eth_init(PHY_ADDRESS, ETH_CLK_060_100MHZ);
    // // // Do any customised PHY configuration here!
    // /* Wait for Link to be established TEMPORARY */
    // while(!phy_link_isup(PHY_ADDRESS)) { // For some reason this does not work right now
    //     vTaskDelay(200);
    //     printf("wait for link\n");
    // }
    // // Enable autonegotiation
    // eth_smi_write(PHY_ADDRESS, PHY_REG_BCR, PHY_REG_BCR_AN);
    // while(!(eth_smi_read(PHY_ADDRESS, PHY_REG_BSR) & PHY_REG_BSR_ANDONE)) {
    //     vTaskDelay(200);
    //     printf("autonegotiate\n");
    // }

    // /* Enable Specific Interrupts */
    // eth_irq_enable(ETH_DMAIER_NISE | ETH_DMAIER_RIE); // Might need to be moved later

    // /* Set Station MAC Address */
    //eth_set_mac(en_mac);

    // eth_desc_init(desc, ETH_TXBUFNB, ETH_RXBUFNB, ETH_TX_BUF_SIZE,
    //         ETH_RX_BUF_SIZE, false);


    // eth_start();
}

/* Test function to send an ethernet packet */
int vSendETH(void) {
    for(uint32_t i=0; i<NELEMS(eth_frame.pkt.dest_mac); i++) {
        eth_frame.pkt.dest_mac[i] = 0xFF;
    }
    for(uint32_t i=0; i<NELEMS(eth_frame.pkt.src_mac); i++) {
        eth_frame.pkt.src_mac[i] = en_mac[i];
    }
    eth_frame.pkt.ethertype = 0x88B5;
    for(int i = 0; i<16; i++) {
        printf("%d,", eth_frame.data[i]);
    }
    printf("\n");
    printf("Link Status: %d\n", phy_link_isup(PHY_ADDRESS));

    return 0; //eth_tx(eth_frame.data, NELEMS(eth_frame.data));
}

/*----------------------------- NEWLIB OVERRIDES -----------------------------*/

/* Redirect 'printf' to UART */
int _write(int file, char * ptr, int len)
{
    int i;

    if (file == 1) {
        for (i = 0; i < len; i++) {
            if (ptr[i] == '\n')
                usart_send_blocking(USART3, '\r');
            usart_send_blocking(USART3, ptr[i]);
        }
        return i;
    }
    return -1;
}

