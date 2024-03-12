/* 09:32 15/03/2023 - change triggering comment */
#include "Pump.h"


#include "utils.h"
#include "PSM.h"
#include "pindef.h"
#include "internal_watchdog.h"

 uint8_t PUMP_RANGE = 100;
 #define ZC_MODE    FALLING
PSM psm(zcPin, dimmerPin, PUMP_RANGE, ZC_MODE, 1, 6);


float flowPerClickAtZeroBar = 0.27f;
int maxPumpClicksPerSecond = 50;
float fpc_multiplier = 1.2f;
bool zcup=false;

float pressureInefficiencyCoefficient[7] ={
  0.045f,
  0.015f,
  0.0033f,
  0.000685f,
  0.000045f,
  0.009f,
  -0.0018f
};



// Initialising some pump specific specs, mainly:
// - max pump clicks(dependant on region power grid spec)
// - pump clicks at 0 pressure in the system
void  pumpInit(unsigned char sensePin, unsigned char controlPin,const int powerLineFrequency, 
const float pumpFlowAtZero) {
 //  psm.freq = powerLineFrequency;

 
  maxPumpClicksPerSecond = powerLineFrequency;
  flowPerClickAtZeroBar = pumpFlowAtZero;
  fpc_multiplier = 60.f / (float)maxPumpClicksPerSecond;



}

// Function that returns the percentage of clicks the pump makes in it's current phase
inline float getPumpPct(const float targetPressure, const float flowRestriction, float currentPressure,float currentFlow,float currentPressureChangeSpeed) {
  if (targetPressure == 0.f) {
      return 0.f;
  }

  float diff = targetPressure - currentPressure;
float clickpersecondforrestrictionflow=getClicksPerSecondForFlow(flowRestriction, currentPressure) ;


  float maxPumpPct = flowRestriction <= 0.f ? 1.f : clickpersecondforrestrictionflow / (float) maxPumpClicksPerSecond;
 


  float pumpPctToMaintainFlow = getClicksPerSecondForFlow(currentFlow, currentPressure) / (float) maxPumpClicksPerSecond;

  if (diff > 2.f) {
    // return fminf(maxPumpPct, 0.25f + 0.2f * diff);
     return fminf(maxPumpPct, 0.25f + 0.4f * diff);
  }

  if (diff > 0.f) {
    return fminf(maxPumpPct, pumpPctToMaintainFlow * 0.95f + 0.1f + 0.2f * diff);
  }

  if (currentPressureChangeSpeed < 0) {
    return fminf(maxPumpPct, pumpPctToMaintainFlow * 0.2f);
  }

  return 0;
}

// Sets the pump output based on a couple input params:
// - live system pressure
// - expected target
// - flow
// - pressure direction
float setPumpPressure(const float targetPressure, const float flowRestriction,
 float currentPressure,float currentFlow,float currentPressureChangeSpeed) {
 //get pump pcent that need to get the targetpressure
 volatile float pumpPct = getPumpPct(targetPressure, flowRestriction, currentPressure,currentFlow, currentPressureChangeSpeed);

  //Serial.println(pumpPct);

   setPumpToRawValue((uint8_t)(pumpPct * PUMP_RANGE));
 return pumpPct;
 

}

void setPumpOff(void) {
  psm.set(0);
  
}

void setPumpFullOn(void) {
  psm.set(PUMP_RANGE);
}

void setPumpToRawValue(const uint8_t val) {
  psm.set(val);
}

void pumpStopAfter(const uint8_t val) {
  psm.stopAfter(val);
}

long getAndResetClickCounter(void) {
  long counter = psm.getCounter();
  psm.resetCounter();
  return counter;
}

int getCPS(void) {
 // watchdogReload();
  unsigned int cps = psm.cps();

 // watchdogReload();
 //cps=100;
  if (cps > 80u) {
    psm.setDivider(2);
    psm.initTimer(cps > 110u ? 60u : 50u);
  }
  else {
    psm.initTimer(cps > 55u ? 60u : 50u);
  }
  return cps;
}

void pumpPhaseShift(void) {
  psm.shiftDividerCounter();
}

// Models the flow per click, follows a compromise between the schematic and recorded findings
// plotted: https://www.desmos.com/calculator/eqynzclagu
float getPumpFlowPerClick(const float pressure) {
  float fpc = 0.f;
  fpc = (pressureInefficiencyCoefficient[5] / pressure + pressureInefficiencyCoefficient[6]) * ( -pressure * pressure ) + ( flowPerClickAtZeroBar - pressureInefficiencyCoefficient[0]) - (pressureInefficiencyCoefficient[1] + (pressureInefficiencyCoefficient[2] - (pressureInefficiencyCoefficient[3] - pressureInefficiencyCoefficient[4] * pressure) * pressure) * pressure) * pressure;
  return fpc * fpc_multiplier;
}

// Follows the schematic from https://www.cemegroup.com/solenoid-pump/e5-60 modified to per-click
float getPumpFlow(const float cps, const float pressure) {
  return cps * getPumpFlowPerClick(pressure);
}

// Currently there is no compensation for pressure measured at the puck, resulting in incorrect estimates
float getClicksPerSecondForFlow(const float flow, const float pressure) {
  if (flow == 0.f) return 0;
  float flowPerClick = getPumpFlowPerClick(pressure);
  float cps = flow / flowPerClick;
  return fminf(cps, (float)maxPumpClicksPerSecond);
}

// Calculates pump percentage for the requested flow and updates the pump raw value
float setPumpFlow(const float targetFlow, const float pressureRestriction, float currentPressure,float currentFlow,float currentPressureChangeSpeed) {
  float pumpPct ;
  // If a pressure restriction exists then the we go into pressure profile with a flowRestriction
  // which is equivalent but will achieve smoother pressure management
  if (pressureRestriction > 0.f && currentPressure > pressureRestriction * 0.5f) {
   pumpPct= setPumpPressure(pressureRestriction, targetFlow, currentPressure,currentFlow,currentPressureChangeSpeed);
  }
  else {
   pumpPct = getClicksPerSecondForFlow(targetFlow, currentPressure) / (float)maxPumpClicksPerSecond;
    setPumpToRawValue(pumpPct * PUMP_RANGE);
  }

  return pumpPct;
}
