#ifndef PORT_FUNCTIONS
#define PORT_FUNCTIONS

/* Function to Initialise all GPIOs */
void vGPIOInitialize();

/* Function to Toggle System LED */
void vSystemLEDToggle();

/* Function to Toggle Status LED */
void vStatusLEDToggle();

/* Function to Toggle Warning LED */
void vWarningLEDToggle();

/* Initialise All System Clock Architecture */
void vConfigureClock();

#endif
