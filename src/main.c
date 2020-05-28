/* MAIN USER APPLICATION */
#include <stdio.h>
#include "main.h"

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

    vConfigureETH(); // Configure Ethernet GPIOs and registers

    for (;;) {
        vWarningLEDToggle();
        #ifdef DEBUG
        printf("Print Task!\n");
        #endif /* DEBUG */
        int result = vSendETH();
        printf("Eth Status: %d\n", result);
        vTaskDelay(1000);
	}
}


int main(void) {

    vConfigureClock();  // Configure RCC, System Clock Tree, PLL, etc...

    vLEDInitialize();   // Set onboard LED GPIOs as outputs

    #ifdef DEBUG
    vConfigureUART();   // Configure UART as output for debugging
    #endif /* DEBUG */

    xTaskCreate(startTask1, "task1", 1024, NULL, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(startTask2, "task2", 1024, NULL, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(startTask3, "task3", 1024, NULL, configMAX_PRIORITIES-1, NULL);

    vTaskStartScheduler();

    // This point is never reached!
    for (;;);
    return 0;
}
