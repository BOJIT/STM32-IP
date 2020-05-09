#include "port_functions.h"

/* Libopencm3 Includes */
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/ethernet/mac.h>
#include <libopencm3/ethernet/phy.h>

/* Global Port-Specific Definitions */
#include "port_config.h"

#ifdef DEBUG
#include <stdio.h>
#endif /* DEBUG */

/*-------------------- PRIVATE DEVICE-SPECIFIC FUNCTIONS ---------------------*/

#ifdef DEBUG
/* Function to Map SYSCLK/4 to GPIO PC9  */
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
uint8_t eth_descs[(ETH_TXBUFNB * ETH_TX_BUF_SIZE) + \
                  (ETH_RXBUFNB * ETH_RX_BUF_SIZE)];

/* TEMPORARY GLOBAL VARIABLES! --------------------------------------------------- */

/* Configure Ethernet Peripheral */
void vConfigureETH() {
    /* Enable relavant clocks */
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOG);

    rcc_periph_clock_enable(RCC_ETHMAC);
    rcc_periph_clock_enable(RCC_ETHMACPTP);
    rcc_periph_clock_enable(RCC_ETHMACRX);
    rcc_periph_clock_enable(RCC_ETHMACTX);

    /* Reset peripheral prior to setting GPIOs */
    rcc_periph_reset_pulse(RCC_ETHMAC);

    /* Configure ethernet GPIOs */
    /* GPIOA */
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_ETH_RMII_MDIO |
            GPIO_ETH_RMII_REF_CLK | GPIO_ETH_RMII_CRS_DV);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP,
            GPIO_OSPEED_100MHZ, GPIO_ETH_RMII_MDIO);
    gpio_set_af(GPIOA, GPIO_AF11, GPIO_ETH_RMII_MDIO |
            GPIO_ETH_RMII_REF_CLK | GPIO_ETH_RMII_CRS_DV);

    /* GPIOB */
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_ETH_RMII_TXD1);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_PP,
            GPIO_OSPEED_100MHZ, GPIO_ETH_RMII_TXD1);
    gpio_set_af(GPIOB, GPIO_AF11, GPIO_ETH_RMII_TXD1);
    // PPS definition to go here when implemented

    /* GPIOC */
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_ETH_RMII_MDC |
            GPIO_ETH_RMII_RXD0 | GPIO_ETH_RMII_RXD1);
    gpio_set_output_options(GPIOC, GPIO_OTYPE_PP,
            GPIO_OSPEED_100MHZ, GPIO_ETH_RMII_MDC);
    gpio_set_af(GPIOC, GPIO_AF11, GPIO_ETH_RMII_MDC |
            GPIO_ETH_RMII_RXD0 | GPIO_ETH_RMII_RXD1);

    /* GPIOG */
    gpio_mode_setup(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE,
            GPIO_ETH_RMII_TX_EN | GPIO_ETH_RMII_TXD0);
    gpio_set_output_options(GPIOG, GPIO_OTYPE_PP,
            GPIO_OSPEED_100MHZ, GPIO_ETH_RMII_TX_EN | GPIO_ETH_RMII_TXD0);
    gpio_set_af(GPIOG, GPIO_AF11, GPIO_ETH_RMII_TX_EN | GPIO_ETH_RMII_TXD0);

    // Probably want to configure NVIC eth_handler here if libopencm3 doesn't handle it

    /* Initialise Ethernet Hardware */
    rcc_periph_reset_pulse(RST_ETHMAC);
    eth_init(PHY_ADDRESS, ETH_CLK_060_100MHZ);

    /* Set Station MAC Address */
    uint8_t en_mac[6] = {1, 2, 3, 4, 5, 6}; // For example purposes only
    eth_set_mac(en_mac);

    eth_desc_init(eth_descs, ETH_TXBUFNB, ETH_RXBUFNB, ETH_TX_BUF_SIZE,
            ETH_RX_BUF_SIZE, false);
    eth_start();
    printf("%d\n", phy_link_isup(PHY_ADDRESS));
}

/* Test function to send an ethernet packet */
void vSendETH(void) {
    uint8_t frame[100];
    eth_tx(frame,sizeof(frame));
}

/*----------------------------- NEWLIB OVERRIDES -----------------------------*/

/* Redirect 'printf' to UART */
int _write(int fd, char *ptr, int len) {
    for(int i = 0; i < len; i++) {
        usart_send_blocking(USART3, *ptr);
        ptr++;
    }
    return len;
}

/*------------------------------- ETHERNET ISR -------------------------------*/

/* Ethernet ISR */
void eth_isr(void) {

}
