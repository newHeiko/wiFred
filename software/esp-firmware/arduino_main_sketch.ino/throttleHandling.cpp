/**
 * This file is part of the wiFred wireless throttle project
 * 
 * It handles the connection to the AVR handling keys, direction switch and speed input
 * 
 * (c) 2018 Heiko Rosemann
 */

#include "locoHandling.h"

/**
 * Send LED settings to AVR - Strings are of the shape "20/100" meaning 20*10ms on time and 100*10ms total cycle time
 */
void setLEDvalues(String led1, String led2, String led3)
{
  if(!locoActive)
  {
    return;
  }
  Serial.println("L1:" + led1);
  Serial.println("L2:" + led2);
  Serial.println("L3:" + led3);
}

