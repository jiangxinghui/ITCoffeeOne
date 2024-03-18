#include <main.h>
#include "Heater.h"
#include "peripherals/peripherals.h"
#include "ADS1X15.h"
#include "internal_watchdog.h"

#include "system_state.h"


void updateProfilerPhases(void) ;

SimpleKalmanFilter smoothPressure(0.6f, 0.6f, 0.1f);
SimpleKalmanFilter smoothPumpFlow(0.1f, 0.1f, 0.01f);
SimpleKalmanFilter smoothConsideredFlow(0.1f, 0.1f, 0.1f);


SensorState currentState;
OPERATION_MODES selectedOperationalMode;

//eepromValues_t runningCfg;



SystemState systemState;

 float pumpPct_Output;

//modbus------------------

#ifdef STM32_BOARD
SoftwareSerial DebugSerial(DebugSerial_RX,DebugSerial_TX);


ModbusRTUSlave modbus(USART_Modbus);  //modbus lib use hardwareserial when stm32

#else


SoftwareSerial blueToothSerial(bt_rxPin,bt_txPin);

ModbusRTUSlave modbus(blueToothSerial);

#endif

uint16_t holdingRegisters[10];
bool coils[2];

//-----------------------------------



Heater myHeater(550);

int cps;

static void heartBeat(void)
{
  uint32_t elapsedTime=millis()-HeartBeatTimer;
  if(elapsedTime>500)
  {
   digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));

  HeartBeatTimer=millis();



  }


}
static void cpsInit() {
  int cps = getCPS();
  // if (cps > 110) { // double 60 Hz
  //   powerLineFrequency = 60u;
  // } else if (cps > 80) { // double 50 Hz
  //   powerLineFrequency = 50u;
  // } else if (cps > 55) { // 60 Hz
  //   powerLineFrequency = 60u;
  // } else if (cps > 0) { // 50 Hz
  //   powerLineFrequency = 50u;
  // }
}

// the setup function runs once when you press reset or power the board
void setup() {

#ifdef STM32_BOARD

#else

#ifdef DEBUG_WITH_AVR
 debug_init();
 #else
Serial.begin(9600);
 #endif

#endif

//pin init
pinInit();


myHeater.gOutputPwr=0;


//pump
setPumpOff();



 // Initialising the saved values or writing defaults if first start


cpsInit();

thermocoupleInit();
adsInit();
pumpInit(zcPin,dimmerPin, 50,0.2225f);

 // updateProfilerPhases();

//modbus init

coils[0]=false;
coils[1]=false;

holdingRegisters[0]=S_TSET*10;  //temp setpoint
holdingRegisters[3]=9*10;  //pressure setpoint
holdingRegisters[7]=2;  //flow setpoint
 modbus.configureHoldingRegisters(holdingRegisters,10);

 modbus.configureCoils(coils, 2);                       // bool array of coil values, number of coils
 //  modbus.configureDiscreteInputs(discreteInputs, 2);     // bool array of discrete input values, number of discrete inputs

  // modbus.configureInputRegisters(inputRegisters, 2);     // unsigned 16 bit integer array of input register values, number of input registers

#ifdef STM32_BOARD
   modbus.begin(1,9600);  //address 1, 9600, n81
   DebugSerial.begin(9600);
#else


   modbus.begin(1,9600,SERIAL_8N1);  //address 1, 9600, n81
#endif



 //iwdcInit();
}









// Function to track time since system has started
static unsigned long getTimeSinceInit(void) {
  static unsigned long startTime = millis();
  return millis() - startTime;
}
// Checks if Brew switch is ON
static bool isSwitchOn(void) {
  return currentState.brewSwitchState ;//&& lcdCurrentPageId == NextionPage::Home;
}
static bool isBoilerFillPhase(unsigned long elapsedTime) {
  return  elapsedTime >= BOILER_FILL_START_TIME;
}

static bool isBoilerFull(unsigned long elapsedTime) {
  bool boilerFull = false;
  if (elapsedTime > BOILER_FILL_START_TIME + 1000UL) {
    boilerFull =  (previousSmoothedPressure - currentState.smoothedPressure > -0.02f)
                &&
                  (previousSmoothedPressure - currentState.smoothedPressure < 0.001f);
  }

  return elapsedTime >= BOILER_FILL_TIMEOUT || boilerFull;
}


static void fillBoilerUntilThreshod(unsigned long elapsedTime) {
  if (elapsedTime >= BOILER_FILL_TIMEOUT) {
    systemState.startupInitFinished = true;
    return;
  }

  if (isBoilerFull(elapsedTime)) {
   // closeValve();
    setPumpOff();
    systemState.startupInitFinished = true;
    return;
  }

  //lcdShowPopup("Filling boiler!");
  //openValve();
  setPumpToRawValue(35);
}


static void fillBoiler(void) {

  if (systemState.startupInitFinished) {
    return;
  }

  if (currentState.temperature > BOILER_FILL_SKIP_TEMP) {
    systemState.startupInitFinished = true;
    return;
  }

  if (isBoilerFillPhase(getTimeSinceInit()) && !isSwitchOn()) {
    fillBoilerUntilThreshod(getTimeSinceInit());
  }
  else if (isSwitchOn()) {
  //  lcdShowPopup("Brew Switch ON!");
  }

}






static void sensorsReadPressure(void) {
  uint32_t elapsedTime = millis() - pressureTimer;   //检测经过的时间间隔


  if (elapsedTime >= GET_PRESSURE_READ_EVERY)
   {  //如果时间间隔到了，就计算一次压力


    float elapsedTimeSec = elapsedTime / 1000.f;

    float result=getPressure();
    if(result!=-1&&result!=-2)
    {
      if(result==0)currentState.pressure=1.0;
      else
     currentState.pressure = result;

    // Serial.println(result);
    previousSmoothedPressure = currentState.smoothedPressure;

    currentState.smoothedPressure = smoothPressure.updateEstimate(currentState.pressure);
    currentState.pressureChangeSpeed = (currentState.smoothedPressure - previousSmoothedPressure) / elapsedTimeSec;
    }
    pressureTimer = millis();
  }
}


static void sensorsReadTemperature(void) {

 if (millis() > thermoTimer) {
    currentState.temperature = thermocoupleRead() ;  //changed by xhjiang
 //currentState.waterTemperature = thermocoupleRead_2() ;  //changed by xhjiang


 //DebugSerial.println(currentState.temperature);

  thermoTimer = millis() + GET_KTYPE_READ_EVERY;


 }

}




static void sensorReadSwitches(void) {
  currentState.brewSwitchState = brewState();
 // currentState.steamSwitchState = steamState();
 // currentState.hotWaterSwitchState = waterPinState() || (currentState.brewSwitchState && currentState.steamSwitchState); // use either an actual switch, or the GC/GCP switch combo
}


static long sensorsReadFlow(float elapsedTimeSec) {
  long pumpClicks = getAndResetClickCounter();

  currentState.pumpClicks = (float) pumpClicks / elapsedTimeSec;
 
  currentState.pumpFlow = getPumpFlow(currentState.pumpClicks, currentState.smoothedPressure);

  previousSmoothedPumpFlow = currentState.pressure;
  // Some flow smoothing
  currentState.smoothedPumpFlow = smoothPumpFlow.updateEstimate(currentState.pumpFlow);

  currentState.pumpFlowChangeSpeed = (currentState.smoothedPumpFlow - previousSmoothedPumpFlow) / elapsedTimeSec;
  return pumpClicks;
}





static void sensorsRead(void){

sensorReadSwitches();

 sensorsReadTemperature();
 sensorsReadPressure();
//  calculateWeightAndFlow();

}




static void brewDetect(void)
{
  if(currentState.brewSwitchState)
  {
    brewActive=true;
  }
  else 
  {
    brewActive=false;

  }
  {
    /* code */
  }
  
}



  static void profiling(void)
  {
    if(brewActive)
    {
     //  CurrentPhase& CurrentPhase=phaseProfiler.getCurrentPhase();

      // // if(phaseProfiler.isFinished())
      // // {
      // //   setPumpOff();
      // //   //closeValve();
      // //   brewActive=false;
      // // }
      // // else 
      
      // if(CurrentPhase.getType()==PHASE_TYPE::PHASE_TYPE_PRESSURE){

     // pumpPct_Output= setPumpPressure(holdingRegisters[3]/10 , holdingRegisters[7], currentState.smoothedPressure,currentState.pumpFlow,currentState.pressureChangeSpeed);
      
      //}
      
      // else
      {

        // float newFlowValue=CurrentPhase.getTarget();
        // float pressureRestriction=CurrentPhase.getRestriction();
        // openvalve();
      pumpPct_Output=  setPumpFlow((double)holdingRegisters[7]/10,holdingRegisters[3]/10,currentState.pressure,currentState.pumpFlow,currentState.pressureChangeSpeed);

        
      }

      
      
      }
      else
      {
  pumpPct_Output=0;
        setPumpOff();

      }

    
    //keep the water at temp
    myHeater.justDoCoffee(holdingRegisters[0]/10, currentState.temperature,currentState.brewSwitchState);


  // myHeater.gTargetTemp=(double)holdingRegisters[0]/10; 
  // myHeater.gInputTemp=(double)holdingRegisters[1]/10;


  // myHeater.loop();

   digitalWrite(HeaterPin,myHeater.heaterState);



  }

static inline void sysHealthCheck(float pressureThreshold)
{

  //Reloading the watchdog timer, if this function fails to run MCU is rebooted
  //watchdogReload();


  /* This *while* is here to prevent situations where the system failed to get a temp reading and temp reads as 0 or -7(cause of the offset)
  If we would use a non blocking function then the system would keep the SSR in HIGH mode which would most definitely cause boiler overheating */
  while (currentState.temperature <= 0.0f || currentState.temperature == NAN || currentState.temperature >= 170.0f) {
    //Reloading the watchdog timer, if this function fails to run MCU is rebooted
    //watchdogReload();
    /* In the event of the temp failing to read while the SSR is HIGH
    we force set it to LOW while trying to get a temp reading - IMPORTANT safety feature */
    setPumpOff();
    //setBoilerOff();
    //setSteamBoilerRelayOff();
    if (millis() > thermoTimer) {
      //LOG_ERROR("Cannot read temp from thermocouple (last read: %.1lf)!", static_cast<double>(currentState.temperature));
     // currentState.steamSwitchState ? lcdShowPopup("COOLDOWN") : lcdShowPopup("TEMP READ ERROR"); // writing a LCD message
      currentState.temperature  = thermocoupleRead();// - runningCfg.offsetTemp;  // Making sure we're getting a value
      thermoTimer = millis() + GET_KTYPE_READ_EVERY;
    }
  }

  // /*Shut down heaters if steam has been ON and unused fpr more than 10 minutes.*/
  // while (currentState.isSteamForgottenON) {
  //   //Reloading the watchdog timer, if this function fails to run MCU is rebooted
  //   //watchdogReload();
  //   //lcdShowPopup("TURN STEAM OFF NOW!");
  //   setPumpOff();
  //   //setBoilerOff();
  //  // setSteamBoilerRelayOff();
  //   currentState.isSteamForgottenON = currentState.steamSwitchState;
  // }



}


void modbusprocess()
{
  //holdingRegisters[0]  //temp target setpoint
  holdingRegisters[1]=currentState.temperature*10;


  holdingRegisters[2]=myHeater.gOutputPwr*10;

//holdingRegisters[3]  //pressure target setpoint
holdingRegisters[4]=currentState.smoothedPressure*10;
 holdingRegisters[5]=currentState.waterTemperature*10;
holdingRegisters[6]=pumpPct_Output*100;
//holdingRegisters[7]= flow setpoint

holdingRegisters[8]=myHeater.osmode;
holdingRegisters[9]=brewState();


      modbus.poll();

    if(coils[0]==true)
    {myHeater.tuning_on();
    coils[0]=false;

   // Serial.println("start tune");

    }

   if(coils[1]==true)
    {myHeater.tuning_off();

   // Serial.println("stop tune");

    coils[1]=false;

    }
}

static void modeSelect(void) {

switch (selectedOperationalMode)
{
case OPERATION_MODES::OPMODE_straight9Bar:
  /* code */


profiling();



  break;

default:
  break;
}



}

void loop()
{
//fillBoiler();

  heartBeat();

double flowsetpoint=(double)analogRead(VR)/100;
if(flowsetpoint<2.0)flowsetpoint=2.0;


  holdingRegisters[7]= flowsetpoint*10;  


  sensorsRead();

 brewDetect();
   modeSelect();

   //sysHealthCheck(SYS_PRESSURE_IDLE);

modbusprocess();





}








