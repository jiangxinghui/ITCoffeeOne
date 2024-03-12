#include <main.h>
#include "Heater.h"
#include "peripherals/peripherals.h"
#include "ADS1X15.h"
#include "internal_watchdog.h"
#include "eeprom_data/eeprom_data.h"
#include "system_state.h"
#include "profiling_phases.h"

void updateProfilerPhases(void) ;

SimpleKalmanFilter smoothPressure(0.6f, 0.6f, 0.1f);
SimpleKalmanFilter smoothPumpFlow(0.1f, 0.1f, 0.01f);
SimpleKalmanFilter smoothConsideredFlow(0.1f, 0.1f, 0.1f);

Profile profile;
PhaseProfiler phaseProfiler{profile};

PredictiveWeight predictiveWeight;
SensorState currentState;
OPERATION_MODES selectedOperationalMode;

eepromValues_t runningCfg;
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
static void cpsInit(eepromValues_t &eepromValues) {
  int cps = getCPS();
  if (cps > 110) { // double 60 Hz
    eepromValues.powerLineFrequency = 60u;
  } else if (cps > 80) { // double 50 Hz
    eepromValues.powerLineFrequency = 50u;
  } else if (cps > 55) { // 60 Hz
    eepromValues.powerLineFrequency = 60u;
  } else if (cps > 0) { // 50 Hz
    eepromValues.powerLineFrequency = 50u;
  }
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
eepromInit();
runningCfg = eepromGetCurrentValues();

cpsInit(runningCfg);

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


void addPhase(PHASE_TYPE type, Transition target, float restriction, int timeMs, float pressureAbove, float pressureBelow, float shotWeight, float isWaterPumped) {
 
 Phase* phase=new Phase
 
{
    .type           = type,
    .target         = target,
    .restriction    = restriction,
    .stopConditions = PhaseStopConditions{ .time=timeMs, .pressureAbove=pressureAbove, .pressureBelow=pressureBelow, .weight=shotWeight, .waterPumpedInPhase=isWaterPumped }
  };

    profile.addPhase(phase );
}


void addPressurePhase(Transition pressure, float flowRestriction, int timeMs, float pressureAbove, float pressureBelow, float shotWeight, float isWaterPumped) {
  addPhase(PHASE_TYPE::PHASE_TYPE_PRESSURE, pressure, flowRestriction, timeMs, pressureAbove, pressureBelow, shotWeight, isWaterPumped);
}

void addFlowPhase(Transition flow, float pressureRestriction, int timeMs, float pressureAbove, float pressureBelow, float shotWeight, float isWaterPumped) {
  addPhase(PHASE_TYPE::PHASE_TYPE_FLOW, flow, pressureRestriction, timeMs, pressureAbove, pressureBelow, shotWeight, isWaterPumped);
}




void addPreinfusionPhases() {
  if (ACTIVE_PROFILE(runningCfg).preinfusionFlowState) { // flow based PI enabled
    float isPressureAbove = ACTIVE_PROFILE(runningCfg).preinfusionPressureAbove ? ACTIVE_PROFILE(runningCfg).preinfusionFlowPressureTarget : -1.f;
    float isWeightAbove = ACTIVE_PROFILE(runningCfg).preinfusionWeightAbove > 0.f ? ACTIVE_PROFILE(runningCfg).preinfusionWeightAbove : -1.f;
    float isWaterPumped = ACTIVE_PROFILE(runningCfg).preinfusionFilled > 0.f ? ACTIVE_PROFILE(runningCfg).preinfusionFilled : -1.f;

    addFlowPhase(Transition{ ACTIVE_PROFILE(runningCfg).preinfusionFlowVol }, ACTIVE_PROFILE(runningCfg).preinfusionFlowPressureTarget, ACTIVE_PROFILE(runningCfg).preinfusionFlowTime * 1000, isPressureAbove, -1, isWeightAbove, isWaterPumped);
  }
  else { // pressure based PI enabled
    // For now handling phase switching on restrictions here but as this grow will have to deal with it otherwise.
    float isPressureAbove = ACTIVE_PROFILE(runningCfg).preinfusionPressureAbove ? ACTIVE_PROFILE(runningCfg).preinfusionBar : -1.f;
    float isWeightAbove = ACTIVE_PROFILE(runningCfg).preinfusionWeightAbove > 0.f ? ACTIVE_PROFILE(runningCfg).preinfusionWeightAbove : -1.f;
    float isWaterPumped = ACTIVE_PROFILE(runningCfg).preinfusionFilled > 0.f ? ACTIVE_PROFILE(runningCfg).preinfusionFilled : -1.f;

    addPressurePhase(Transition{ ACTIVE_PROFILE(runningCfg).preinfusionBar }, ACTIVE_PROFILE(runningCfg).preinfusionPressureFlowTarget, ACTIVE_PROFILE(runningCfg).preinfusionSec * 1000, isPressureAbove, -1, isWeightAbove, isWaterPumped);
  }
}

void addMainExtractionPhasesAndRamp() {
  int rampPhaseIndex = -1;

  if (ACTIVE_PROFILE(runningCfg).profilingState) {
    if (ACTIVE_PROFILE(runningCfg).tpState) {
      // ----------------- Transition Profile ----------------- //
      if (ACTIVE_PROFILE(runningCfg).tpType) { // flow based profiling enabled
        /* Setting the phase specific restrictions */
        /* ------------------------------------------ */
        float fpStart = ACTIVE_PROFILE(runningCfg).tfProfileStart;
        float fpEnd = ACTIVE_PROFILE(runningCfg).tfProfileEnd;
        uint16_t fpHold = ACTIVE_PROFILE(runningCfg).tfProfileHold * 1000;
        float holdLimit = ACTIVE_PROFILE(runningCfg).tfProfileHoldLimit > 0.f ? ACTIVE_PROFILE(runningCfg).tfProfileHoldLimit : -1;
        TransitionCurve curve = (TransitionCurve)ACTIVE_PROFILE(runningCfg).tfProfileSlopeShape;
        uint16_t curveTime = ACTIVE_PROFILE(runningCfg).tfProfileSlope * 1000;
        /* ------------------------------------------ */

        if (fpStart > 0.f && fpHold > 0) {
          addFlowPhase(Transition{ fpStart }, holdLimit, fpHold, -1, -1, -1, -1);
          rampPhaseIndex = rampPhaseIndex > 0 ? rampPhaseIndex : profile.phaseCount() - 1;
        }
        addFlowPhase(Transition{ fpStart, fpEnd, curve, curveTime }, ACTIVE_PROFILE(runningCfg).tfProfilingPressureRestriction, curveTime, -1, -1, -1, -1);
        rampPhaseIndex = rampPhaseIndex > 0 ? rampPhaseIndex : profile.phaseCount() - 1;
      }
      else { // pressure based profiling enabled
        /* Setting the phase specific restrictions */
        /* ------------------------------------------ */
        float ppStart = ACTIVE_PROFILE(runningCfg).tpProfilingStart;
        float ppEnd = ACTIVE_PROFILE(runningCfg).tpProfilingFinish;
        uint16_t ppHold = ACTIVE_PROFILE(runningCfg).tpProfilingHold * 1000;
        float holdLimit = ACTIVE_PROFILE(runningCfg).tpProfilingHoldLimit > 0.f ? ACTIVE_PROFILE(runningCfg).tpProfilingHoldLimit : -1;
        TransitionCurve curve = (TransitionCurve)ACTIVE_PROFILE(runningCfg).tpProfilingSlopeShape;
        uint16_t curveTime = ACTIVE_PROFILE(runningCfg).tpProfilingSlope * 1000;
        /* ------------------------------------------ */

        if (ppStart > 0.f && ppHold > 0) {
          addPressurePhase(Transition{ ppStart }, holdLimit, ppHold, -1, -1, -1, -1);
          rampPhaseIndex = rampPhaseIndex > 0 ? rampPhaseIndex : profile.phaseCount() - 1;
        }
        addPressurePhase(Transition{ ppStart, ppEnd, curve, curveTime }, ACTIVE_PROFILE(runningCfg).tpProfilingFlowRestriction, curveTime, -1, -1, -1, -1);
        rampPhaseIndex = rampPhaseIndex > 0 ? rampPhaseIndex : profile.phaseCount() - 1;
      }
    }

    // ----------------- Main Profile ----------------- //
    if (ACTIVE_PROFILE(runningCfg).mfProfileState) { // flow based profiling enabled
      /* Setting the phase specific restrictions */
      /* ------------------------------------------ */
      float fpStart = ACTIVE_PROFILE(runningCfg).mfProfileStart;
      float fpEnd = ACTIVE_PROFILE(runningCfg).mfProfileEnd;
      TransitionCurve curve = (TransitionCurve)ACTIVE_PROFILE(runningCfg).mfProfileSlopeShape;
      uint16_t curveTime = ACTIVE_PROFILE(runningCfg).mfProfileSlope * 1000;

      /* ------------------------------------------ */
      addFlowPhase(Transition(fpStart, fpEnd, curve, curveTime), ACTIVE_PROFILE(runningCfg).mfProfilingPressureRestriction, -1, -1, -1, -1, -1);
    }
    else { // pressure based profiling enabled
      /* Setting the phase specific restrictions */
      /* ------------------------------------------ */
      float ppStart = ACTIVE_PROFILE(runningCfg).mpProfilingStart;
      float ppEnd = ACTIVE_PROFILE(runningCfg).mpProfilingFinish;
      TransitionCurve curve = (TransitionCurve)ACTIVE_PROFILE(runningCfg).mpProfilingSlopeShape;
      uint16_t curveTime = ACTIVE_PROFILE(runningCfg).mpProfilingSlope * 1000;
      /* ------------------------------------------ */
      addPressurePhase(Transition(ppStart, ppEnd, curve, curveTime), ACTIVE_PROFILE(runningCfg).mpProfilingFlowRestriction, -1, -1, -1, -1, -1);
    }
  } else { // Shot profiling disabled. Default to 9 bars
    addPressurePhase(Transition(9.f), -1, -1, -1, -1, -1, -1);
  }

  rampPhaseIndex = rampPhaseIndex > 0 ? rampPhaseIndex : profile.phaseCount() - 1;
 // insertRampPhaseIfNeeded(rampPhaseIndex);
}





 void updateProfilerPhases(void) {
  float shotTarget = -1.f;

  if (ACTIVE_PROFILE(runningCfg).stopOnWeightState) {
    shotTarget = (ACTIVE_PROFILE(runningCfg).shotStopOnCustomWeight < 1.f)
      ? ACTIVE_PROFILE(runningCfg).shotDose * ACTIVE_PROFILE(runningCfg).shotPreset
      : ACTIVE_PROFILE(runningCfg).shotStopOnCustomWeight;
  }

  //update global stop conditions (currently only stopOnWeight is configured in nextion)
  profile.globalStopConditions = GlobalStopConditions{ .weight=shotTarget };

  profile.clear();

  //Setup release pressure + fill@7ml/sec
  if (runningCfg.basketPrefill) {
  //  addFillBasketPhase(7.f);
  }

  // Setup pre-infusion if needed
  if (ACTIVE_PROFILE(runningCfg).preinfusionState) {
    addPreinfusionPhases();
  }

  // Setup the soak phase if neecessary
  if (ACTIVE_PROFILE(runningCfg).soakState) {
  //  addSoakPhase();
  }
  preInfusionFinishedPhaseIdx = profile.phaseCount();

  addMainExtractionPhasesAndRamp();
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


static void calculateWeightAndFlow(void) {
  uint32_t elapsedTime = millis() - flowTimer;

  if (brewActive) {
    // Marking for tare in case smth has gone wrong and it has exited tare already.
    if (currentState.weight < -.3f) currentState.tarePending = true;

    if (elapsedTime > REFRESH_FLOW_EVERY) {
      flowTimer = millis();
      float elapsedTimeSec = elapsedTime / 1000.f;
      long pumpClicks = sensorsReadFlow(elapsedTimeSec);
      float consideredFlow = currentState.smoothedPumpFlow * elapsedTimeSec;
      // Update predictive class with our current phase
      
      CurrentPhase& phase = phaseProfiler.getCurrentPhase();
      predictiveWeight.update(currentState, phase, runningCfg);

      // Start the predictive weight calculations when conditions are true
      if (predictiveWeight.isOutputFlow() || currentState.weight > 0.4f) {
        float flowPerClick = getPumpFlowPerClick(currentState.smoothedPressure);
        float actualFlow = (consideredFlow > pumpClicks * flowPerClick) ? consideredFlow : pumpClicks * flowPerClick;
        /* Probabilistically the flow is lower if the shot is just started winding up and we're flow profiling,
        once pressure stabilises around the setpoint the flow is either stable or puck restriction is high af. */
        //todo: later
        if ((ACTIVE_PROFILE(runningCfg).mfProfileState || ACTIVE_PROFILE(runningCfg).tpType) && currentState.pressureChangeSpeed > 0.15f) {
          if ((currentState.smoothedPressure < ACTIVE_PROFILE(runningCfg).mfProfileStart * 0.9f)
          || (currentState.smoothedPressure < ACTIVE_PROFILE(runningCfg).tfProfileStart * 0.9f)) {
            actualFlow *= 0.3f;
          }
        }
        currentState.consideredFlow = smoothConsideredFlow.updateEstimate(actualFlow);
        currentState.shotWeight = currentState.scalesPresent ? currentState.shotWeight : currentState.shotWeight + actualFlow;
      }
      currentState.waterPumped += consideredFlow;
    }
  } else {
    currentState.consideredFlow = 0.f;
    currentState.pumpClicks = getAndResetClickCounter();
    flowTimer = millis();
  }
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
       CurrentPhase& CurrentPhase=phaseProfiler.getCurrentPhase();

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








