#include "port_functions.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/pwr.h>

#include "port_config.h"

/* Function to Initialise all GPIOs */
void vGPIOInitialize() {
    /* Enable GPIOC clock. */
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
    rcc_config.plln = SYSCLK_FREQ/500000;
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
    rcc_clock_setup_hse(&rcc_config, HSE_FREQ);
}
