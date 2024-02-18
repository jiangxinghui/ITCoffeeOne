#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include "pindef.h"
#include "peripherals.h"
#include <Arduino.h>


//Function to get the state of the brew switch button
//returns true or false based on the read P(power) value
static inline bool brewState(void) {
  return digitalRead(brewPin) == LOW; // pin will be low when switch is ON.
}

#endif