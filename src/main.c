/**
 * @file
 * Main entry point for the program
 *
 */

#ifdef DEBUG
#include <stdio.h>
#endif /* DEBUG */

#include <lwip/api.h>
#include <string.h>

/* Inclue FreeRTOS Headers */
#include "FreeRTOS.h"
#include "task.h"

/* Include FreeRTOS Tasks and Config */
#include "FreeRTOSConfig.h"

/* Include Device-Specific Functions */
#include "port.h"
#include "port_ethernetif.h"

/* Stack Overflow Handler - move elsewhere */
extern void vApplicationStackOverflowHook(
    xTaskHandle *pxTask,
    signed portCHAR *pcTaskName);

void vApplicationStackOverflowHook(
  xTaskHandle *pxTask __attribute((unused)),
  signed portCHAR *pcTaskName __attribute((unused))
) {
    for(;;);    // Loop forever here..
}

/* Task 1 - Blink System LED */
void startTask1(void *args __attribute((unused))) {

    for (;;) {
        vSystemLEDToggle();
        vTaskDelay(100);
	}
}

/* Task 2 - Blink Status LED */
void startTask2(void *args __attribute((unused))) {

    for (;;) {
        vStatusLEDToggle();
        vTaskDelay(500);
	}
}

/* Task 3 - Blink Warning LED */
void startTask3(void *args __attribute((unused))) {

    networkInit(); // Configure Ethernet GPIOs and registers

    struct netconn *conn;
    char msg[] = "alpha";
    struct netbuf *buf;
    char * data;

    conn = netconn_new(NETCONN_UDP);
    netconn_bind(conn, IP_ADDR_ANY, 1234); //local port

    netconn_connect(conn, IP_ADDR_BROADCAST, 1235);

    for (;;) {
        vWarningLEDToggle();
        buf = netbuf_new();
        data =netbuf_alloc(buf, sizeof(msg));
        memcpy(data, msg, sizeof(msg));
        netconn_send(conn, buf);
        netbuf_delete(buf); // De-allocate packet buffer
        vTaskDelay(1000);
	}
}

/** 
 * @brief This is the main entry point to the program
*/
int main(void) {

    vConfigureClock();  // Configure RCC, System Clock Tree, PLL, etc...

    vLEDInitialize();   // Set onboard LED GPIOs as outputs

    #ifdef DEBUG
    portUartInit(115200);   // Configure UART as output for debugging
    #endif /* DEBUG */

    xTaskCreate(startTask1, "task1", 350, NULL, 5, NULL);
    xTaskCreate(startTask2, "task2", 350, NULL, 5, NULL);
    xTaskCreate(startTask3, "lwip", 1024, NULL, configMAX_PRIORITIES-2, NULL);

    vTaskStartScheduler();

    /* This point is never reached, as the scheduler is blocking! */
    for (;;);
    return 0;
}
