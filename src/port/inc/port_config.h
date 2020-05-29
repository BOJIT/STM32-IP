#ifndef PORT_CONFIG
#define PORT_CONFIG

/* Enable Debug Options */
#define DEBUG

/* Global Clock Configuration */
#define HSE_FREQ         8000000 // 8 MHz External Crystal
#define SYSCLK_FREQ      96000000 // 96 MHz System Frequency

/* LED Pins (can be mapped by user to any other pin) */
#define SYSTEM_LED_PORT     GPIOB
#define SYSTEM_LED_PIN      GPIO0
#define SYSTEM_LED_RCC      RCC_GPIOB
#define STATUS_LED_PORT     GPIOB
#define STATUS_LED_PIN      GPIO7
#define STATUS_LED_RCC      RCC_GPIOB
#define WARNING_LED_PORT    GPIOB
#define WARNING_LED_PIN     GPIO14
#define WARNING_LED_RCC     RCC_GPIOB

/* UART DEBUG Pins (should be on the same port) */
#ifdef DEBUG
#define DEBUG_UART_PORT     GPIOD
#define DEBUG_UART_TX       GPIO8
#define DEBUG_UART_RX       GPIO9
#define DEBUG_UART_RCC      RCC_GPIOD
#endif /* DEBUG */

/* Ethernet Pins and Configuration */
#define GPIO_ETH_RMII_MDIO GPIO2    /* PA2 */
#define GPIO_ETH_RMII_MDC GPIO1     /* PC1 */
//#define GPIO_ETH_RMII_PPS_OUT GPIO5 /* PB5 */ USE LATER
#define GPIO_ETH_RMII_TX_EN GPIO11  /* PG11 */
#define GPIO_ETH_RMII_TXD0 GPIO12   /* PG13 */
#define GPIO_ETH_RMII_TXD1 GPIO13   /* PB13 */
#define GPIO_ETH_RMII_REF_CLK GPIO1 /* PA1 */
#define GPIO_ETH_RMII_CRS_DV GPIO7  /* PA7 */
#define GPIO_ETH_RMII_RXD0 GPIO4    /* PC4 */
#define GPIO_ETH_RMII_RXD1 GPIO5    /* PC5 */
/* Note that the ports themselves are defined in vConfigureETH()! */

/* Definitions for Ethernet Descriptor Buffers */
// #define ETH_TXBUFNB 4
// #define ETH_RXBUFNB 4
// #define ETH_TX_BUF_SIZE (1520 + 16)
// #define ETH_RX_BUF_SIZE (1520 + 16)

#define PHY_ADDRESS PHY0

/* Handy Macros */
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

#endif /* PORT_CONFIG */
