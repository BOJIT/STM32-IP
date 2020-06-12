/**
 * @file
 * @brief Port-specific application configuration
 * 
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 7 June 2020
 */

#ifndef __PORT_CONFIG__
#define __PORT_CONFIG__

/* Configuration includes */
#include <global_config.h>

/*----------------------------------------------------------------------------*/

/* Clock configuration */
#define HSE_FREQ         8000000    ///< MCU input oscillator frequency
#define SYSCLK_FREQ      96000000   ///< Core MCU system frequency

/*----------------------------------------------------------------------------*/

/* LED configuration */
#define SYSTEM_LED_PORT     GPIOB       ///< GPIO Port for System LED
#define SYSTEM_LED_PIN      GPIO0       ///< GPIO Pin for System LED
#define SYSTEM_LED_RCC      RCC_GPIOB   ///< RCC (clock enable) for System LED
#define STATUS_LED_PORT     GPIOB       ///< GPIO Port for Status LED
#define STATUS_LED_PIN      GPIO7       ///< GPIO Pin for StatusLED
#define STATUS_LED_RCC      RCC_GPIOB   ///< RCC (clock enable) for Status LED
#define WARNING_LED_PORT    GPIOB       ///< GPIO Port for Warning LED
#define WARNING_LED_PIN     GPIO14      ///< GPIO Pin for Status LED
#define WARNING_LED_RCC     RCC_GPIOB   ///< RCC (clock enable) for Warning LED

/*----------------------------------------------------------------------------*/

#ifdef DEBUG
/* Serial configuration */
#define DEBUG_UART_PORT     GPIOD     ///< GPIO Port for UART Rx <b>AND</b> TX!
#define DEBUG_UART_TX       GPIO8     ///< GPIO Pin for Tx Pin
#define DEBUG_UART_RX       GPIO9     ///< GPIO Pin for Rx Pin
#define DEBUG_UART_RCC      RCC_GPIOD ///< RCC (clock enable) for UART
#endif /* DEBUG */

/*----------------------------------------------------------------------------*/

/* Ethernet configuration */
#define PHY_ADDRESS         PHY0        ///< PHY Address for SMI bus
#define PHY_LAN8742A    ///< PHY chipset

//#define MAC_ADDR_MANUAL  ///< If left undefined a PR address will be created

#ifdef MAC_ADDR_MANUAL
    #define MAC_ADDR_0      0x00
    #define MAC_ADDR_1      0x80
    #define MAC_ADDR_2      0xE1
    #define MAC_ADDR_3      0x01
    #define MAC_ADDR_4      0x02
    #define MAC_ADDR_5      0x03
#endif /* MAC_ADDRESS_MANUAL */


/* Static IP configuration - only required if LWIP_DHCP==0 */
#define LWIP_IP_0   192
#define LWIP_IP_1   168
#define LWIP_IP_2   50
#define LWIP_IP_3   51

#define LWIP_NM_0   255
#define LWIP_NM_1   255
#define LWIP_NM_2   255
#define LWIP_NM_3   0

#define LWIP_GW_0   192
#define LWIP_GW_1   168
#define LWIP_GW_2   50
#define LWIP_GW_3   1


#define LWIP_HOSTNAME       "lwip"  ///< Hostname of lwIP netif

/* Pinout */
#define GPIO_ETH_RMII_MDIO      GPIO2    /* PA2 THESE MIGHT MOVE*/
#define GPIO_ETH_RMII_MDC       GPIO1    /* PC1 */
#define GPIO_ETH_RMII_PPS_OUT   GPIO5    /* PB5 */
#define GPIO_ETH_RMII_TX_EN     GPIO11   /* PG11 */
#define GPIO_ETH_RMII_TXD0      GPIO13   /* PG13 */
#define GPIO_ETH_RMII_TXD1      GPIO13   /* PB13 */
#define GPIO_ETH_RMII_REF_CLK   GPIO1    /* PA1 */
#define GPIO_ETH_RMII_CRS_DV    GPIO7    /* PA7 */
#define GPIO_ETH_RMII_RXD0      GPIO4    /* PC4 */
#define GPIO_ETH_RMII_RXD1      GPIO5    /* PC5 */

/*----------------------------------------------------------------------------*/

#endif /* __PORT_CONFIG__ */
