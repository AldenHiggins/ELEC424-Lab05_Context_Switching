#ifndef SYS_INIT_H_
#define SYS_INIT_H_

/*
 * MySystemInit is used to set default system initializations and configure system * clock to 72MHz.
*/
void init_system_clk(void);

/*
 * Initializes pin PB5 which is connected to the green LED. 
 * Port mode is push-pull output.
 */
void init_blink(void);

/*
 * Initializes pins connected to motors and configures respective timers 
 * to output PWM signals.
 */
void init_motors(void);

#endif
