/**
 * @file
 * Contains declarations of all the public functions that must be provided by
 * the architecture-specific port. All functions should start with the word
 * 'port' so as to make it clear which parts of the application are
 * architecture-specific (<b>_write</b> is the only exception to this rule).
 *
 */

#ifndef __PORT__
#define __PORT__

/*----------------------------- PUBLIC FUNCTIONS -----------------------------*/

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

/** 
 * @brief Initialise the UART that is designated for the debug interface.
 * @param baud Sets the baud-rate of the UART
*/
void portUartInit(int baud);


void networkInit(void);

/*----------------------------- NEWLIB OVERRIDES -----------------------------*/

/** 
 * @brief Overrides the <b>newlib</b> "_write" function that is used by 
 * <b>printf()</b>.
 * @param fd file descriptor - handled by <b>newlib</b>.
 * @param ptr pointer to char array - handled by <b>newlib</b>. 
 * @param len length of char array - handled by <b>newlib</b>. 
 * @retval length of char array, or -1 on failure.
*/
int _write(int fd, char *ptr, int len);

#endif /* __PORT__ */
