#ifndef THERMOCOUPLE_H
#define THERMOCOUPLE_H


#include "pindef.h"
#if defined SINGLE_BOARD
#include <Adafruit_MAX31855.h>
SPIClass thermoSPI(thermoDI, thermoDO, thermoCLK);
Adafruit_MAX31855 thermocouple(thermoCS, &thermoSPI);
#else
#include <max6675.h>

 MAX6675 max6675(thermoCLK, thermoCS, thermoDO);
 //MAX6675 max6675_2(thermoCLK_2, thermoCS_2, thermoDO_2);
#endif

static inline void thermocoupleInit(void) {

}


static inline float thermocoupleRead(void) {
  return max6675.readCelsius();
}
// static inline float thermocoupleRead_2(void) {
//   return max6675_2.readCelsius();
// }
#endif
