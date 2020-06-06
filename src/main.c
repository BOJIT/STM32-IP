/* MAIN USER APPLICATION */
#include <stdio.h>
#include "main.h"

#include <lwip/api.h>
#include <string.h>

/* Stack Overflow Handler */
extern void vApplicationStackOverflowHook(
    xTaskHandle *pxTask,
    signed portCHAR *pcTaskName);

void vApplicationStackOverflowHook(
  xTaskHandle *pxTask __attribute((unused)),
  signed portCHAR *pcTaskName __attribute((unused))
) {
    for(;;);    // Loop forever here..
}

void gdb_break() {
    // empty - convenient tag for GDB
}

/* Task 1 - Blink System LED */
void startTask1(void *args __attribute((unused))) {

    for (;;) {
        vSystemLEDToggle();
        vTaskDelay(50);
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
    char msg1[] = "alpha";
    char msg2[] = "beta";
    struct netbuf *buf;
    char * data;

    conn = netconn_new(NETCONN_UDP);
    netconn_bind(conn, IP_ADDR_ANY, 1234); //local port

    netconn_connect(conn, IP_ADDR_BROADCAST, 1235);

    for (;;) {
        vWarningLEDToggle();
        buf = netbuf_new();
        data =netbuf_alloc(buf, sizeof(msg1));
        memcpy(data, msg1, sizeof(msg1));
        netconn_send(conn, buf);
        netbuf_delete(buf); // De-allocate packet buffer
        buf = netbuf_new();
        data =netbuf_alloc(buf, sizeof(msg2));
        memcpy(data, msg2, sizeof(msg2));
        netconn_send(conn, buf);
        netbuf_delete(buf); // De-allocate packet buffer
        #ifdef DEBUG
        printIP();
        #endif /* DEBUG */
        vTaskDelay(1000);
        gdb_break();
	}
}


int main(void) {

    vConfigureClock();  // Configure RCC, System Clock Tree, PLL, etc...

    vLEDInitialize();   // Set onboard LED GPIOs as outputs

    #ifdef DEBUG
    vConfigureUART();   // Configure UART as output for debugging
    #endif /* DEBUG */

    xTaskCreate(startTask1, "task1", 350, NULL, 5, NULL);
    xTaskCreate(startTask2, "task2", 350, NULL, 5, NULL);
    xTaskCreate(startTask3, "lwip", 1024, NULL, configMAX_PRIORITIES-2, NULL);

    vTaskStartScheduler();

    // This point is never reached!
    for (;;);
    return 0;
}
