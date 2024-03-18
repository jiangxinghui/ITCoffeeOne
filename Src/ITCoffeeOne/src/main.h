#include <Arduino.h>
#include <SimpleKalmanFilter.h>

#include "peripherals/thermocouple.h"
#include "sensors_state.h"


#include <SoftwareSerial.h>
#include <ModbusRTUSlave.h>
#include <max6675.h>
#include "peripherals/pressure_sensor.h"
#include "ModbusRTUSlave.h"
#include "Pump.h"



#if defined STM32_BOARD
    #define GET_KTYPE_READ_EVERY    500 // max31855 amp module data read interval not recommended to be changed to lower than 250 (ms)

#else


//#include <avr8-stub.h>

    #define GET_KTYPE_READ_EVERY    250 // max6675 amp module data read interval not recommended to be changed to lower than 250 (ms)
#endif

#define GET_PRESSURE_READ_EVERY 250 // Pressure refresh interval (ms)
#define REFRESH_FLOW_EVERY      50 // Flow refresh interval (ms)
#define BOILER_FILL_SKIP_TEMP   85.f // Boiler fill skip temperature threshold
#define BOILER_FILL_START_TIME  3000UL // Boiler fill start time - 3 sec since system init.
#define BOILER_FILL_TIMEOUT     8000UL // Boiler fill timeout - 8sec since system init.
#define BOILER_FILL_SKIP_TEMP   85.f // Boiler fill skip temperature threshold
#define SYS_PRESSURE_IDLE       0.7f // System pressure threshold at idle

enum class OPERATION_MODES {
  OPMODE_straight9Bar,
  OPMODE_justPreinfusion,
  OPMODE_justPressureProfile,
  OPMODE_manual,
  OPMODE_preinfusionAndPressureProfile,
  OPMODE_flush,
  OPMODE_descale,
  OPMODE_flowPreinfusionStraight9BarProfiling,
  OPMODE_justFlowBasedProfiling,
  OPMODE_steam,
  OPMODE_FlowBasedPreinfusionPressureBasedProfiling,
  OPMODE_everythingFlowProfiled,
  OPMODE_pressureBasedPreinfusionAndFlowProfile
} ;

//timers

unsigned long thermoTimer;


unsigned long HeartBeatTimer;  //added by xhjiang
unsigned long pressureTimer;

unsigned long brewingTimer;
unsigned long flowTimer;
unsigned long steamTime;


//brew detection vars
bool brewActive = false;
bool nonBrewModeActive = false;
//PP&PI variables
int preInfusionFinishedPhaseIdx = 3;
bool homeScreenScalesEnabled = false;

//other util vars
float previousSmoothedPressure;
float previousSmoothedPumpFlow;

