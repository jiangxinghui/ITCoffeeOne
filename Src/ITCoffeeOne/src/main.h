#include <Arduino.h>


#include "peripherals/thermocouple.h"
#include "sensors_state.h"


#include <SoftwareSerial.h>
#include <ModbusRTUSlave.h>
#include <max6675.h>

#include "ModbusRTUSlave.h"



#if defined STM32_BOARD
    #define GET_KTYPE_READ_EVERY    70 // max31855 amp module data read interval not recommended to be changed to lower than 70 (ms)
#include "STM32FreeRTOS.h"

#else
#include <Arduino_FreeRTOS.h>


    #define GET_KTYPE_READ_EVERY    250 // max6675 amp module data read interval not recommended to be changed to lower than 250 (ms)
#endif




unsigned long thermoTimer;