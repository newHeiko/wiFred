/**
 * This file is part of the wiFred wireless throttle project
 * 
 * It handles the connection to the AVR handling keys, direction switch and speed input
 * 
 * (c) 2018 Heiko Rosemann
 */

#ifndef _THROTTLE_HANDLING_H_
#define _THROTTLE_HANDLING_H_

/**
 * Send LED settings to AVR - Strings are of the shape "20/100" meaning 20*10ms on time and 100*10ms total cycle time
 */
void setLEDvalues(String led1, String led2, String led3);

/**
 * Periodically check serial port for new information from the AVR
 * 
 * Return a string to be sent to wiThrottle server, may include multiple newlines
 */
String handleThrottle(void);

/**
 * Set current throttle status to ESTOP
 */
void setESTOP(void);

/**
 * Get current direction - returns true when reverse
 */
bool getReverse(void);

#endif
