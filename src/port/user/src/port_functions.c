#include "port_functions.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/usart.h>

#include "port_config.h"

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

/* Function to Initialise all GPIOs */
void vLEDInitialize() {
    /* Enable GPIOB clock. */
    rcc_periph_clock_enable(RCC_GPIOB);

    /* System LED */
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
    /* Status LED */
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);
    /* Warning LED */
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO14);
}

/* Function to Toggle System LED */
void vSystemLEDToggle() {
    gpio_toggle(GPIOB, GPIO0);
}

/* Function to Toggle Status LED */
void vStatusLEDToggle() {
    gpio_toggle(GPIOB, GPIO7);
}

/* Function to Toggle Warning LED */
void vWarningLEDToggle() {
    gpio_toggle(GPIOB, GPIO14);
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
    #endif
}

/* Configure UART for debugging messages */
void vConfigureUART() {
    /* Enable GPIOD and USART3 clock. */
    rcc_periph_clock_enable(RCC_GPIOD);
    rcc_periph_clock_enable(RCC_USART3);
    /* Setup GPIO pins for USART3 transmit. */
    gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8 | GPIO9);
    gpio_set_af(GPIOD, GPIO_AF7, GPIO8 | GPIO9);

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

/* Redirect 'printf' to UART */
int _write(int fd, char *ptr, int len) {
    for(int i = 0; i < len; i++) {
        usart_send_blocking(USART3, *ptr);
        ptr++;
    }
    return len;
}
