#ifndef THERMOCOUPLE_H
#define THERMOCOUPLE_H


#include "pindef.h"

#include <max6675.h>
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

static inline void thermocoupleInit(void)
{
  

}

static inline float thermocoupleRead(void) {
  return thermocouple.readCelsius();
}


#endif
