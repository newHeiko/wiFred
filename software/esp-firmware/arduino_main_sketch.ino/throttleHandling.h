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

#endif
