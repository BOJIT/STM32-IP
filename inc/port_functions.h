#ifndef PORT_FUNCTIONS
#define PORT_FUNCTIONS

/* Function to Initialise all GPIOs */
void vLEDInitialize();

/* Function to Toggle System LED */
void vSystemLEDToggle();

/* Function to Toggle Status LED */
void vStatusLEDToggle();

/* Function to Toggle Warning LED */
void vWarningLEDToggle();

/* Initialise All System Clock Architecture */
void vConfigureClock();

/* Configure UART for debugging messages */
void vConfigureUART();

/* Redirect 'printf' to UART */
int _write(int fd, char *ptr, int len);

#endif
