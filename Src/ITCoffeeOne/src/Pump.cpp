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





/// @brief 获取 达到目标压力 泵需要工作的百分比
/// @param targetPressure 目标压力
/// @param flowRestriction 流量限制值。 
/// @param currentPressure 当前压力
/// @param currentFlow 当前流量
/// @param currentPressureChangeSpeed 当前压力变化速度
/// @return 
inline float getPumpPct(const float targetPressure, const float flowRestriction, float currentPressure,float currentFlow,float currentPressureChangeSpeed) {
 //如果目标压力为零，表示泵不用工作，返回0
  if (targetPressure == 0.f) {
      return 0.f;
  }

  float diff = targetPressure - currentPressure;

  //根据流量限制和当前压力，计算出达到流量限制的 每秒震动次数
float clickpersecondforrestrictionflow=getClicksPerSecondForFlow(flowRestriction, currentPressure) ;

//达到限制流量，的最大泵工作百分比
  float maxPumpPct = flowRestriction <= 0.f ? 1.f : clickpersecondforrestrictionflow / (float) maxPumpClicksPerSecond;
 

//根据当前流量和压力，得出维持流量的泵的pumpPct
  float pumpPctToMaintainFlow = getClicksPerSecondForFlow(currentFlow, currentPressure) / (float) maxPumpClicksPerSecond;

//如果压力差距很大，
  if (diff > 2.f) {

     return fminf(maxPumpPct, 0.25f + 0.2f * diff);
  }
//如果压力差大于0
  if (diff > 0.f) {
    return fminf(maxPumpPct, pumpPctToMaintainFlow * 0.95f + 0.1f + 0.2f * diff);
  }

//如果压力超过了设定

//diff<0

  //如果压力增大正在减缓，维持一个最小流量
  if (currentPressureChangeSpeed < 0) {
    return fminf(maxPumpPct, pumpPctToMaintainFlow * 0.2f+0.1f);
  }
  //压力超过了，并且正在增加，
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
//added by itcoffee
if(pumpPct<0)pumpPct=0;

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

  unsigned int cps;
 
 
 cps=   psm.cps();

 // watchdogReload();
 
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

/// @brief 在特定压力下，根据一个 泵的流量压力特性曲线，获取每次震动带来的流量。
/// @param pressure 给定压力
/// @return 每次震动的流量
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

/// @brief 给定压力下，达到目标流量需要每秒震动次数
/// @param flow 目标流量
/// @param pressure 当前压力
/// @return 
float getClicksPerSecondForFlow(const float flow, const float pressure) {
 //如果目标流量为零，不需要工作
  if (flow == 0.f) return 0;
  //获取当前压力下，震动一次的流量
  float flowPerClick = getPumpFlowPerClick(pressure);
  //目标流量所需要的震动次数
  float cps = flow / flowPerClick;
  //约束最大震动次数为泵的最大震动次数
  return fminf(cps, (float)maxPumpClicksPerSecond);
}

// Calculates pump percentage for the requested flow and updates the pump raw value
float setPumpFlow(const float targetFlow, const float pressureRestriction, float currentPressure,float currentFlow,float currentPressureChangeSpeed) {
  float pumpPct ;
  // If a pressure restriction exists then the we go into pressure profile with a flowRestriction
  // which is equivalent but will achieve smoother pressure management


  //如果有压力限制，那么在目标流量作为最大流量的约束下，达到目标压力需要的泵perct
  if (pressureRestriction > 0.f && currentPressure > pressureRestriction * 0.5f) {
   pumpPct= setPumpPressure(pressureRestriction, targetFlow, currentPressure,currentFlow,currentPressureChangeSpeed);
  }
  else {

    //否则，不管压力约束，在当前压力情况下达到目标流量的泵工作perct
   pumpPct = getClicksPerSecondForFlow(targetFlow, currentPressure) / (float)maxPumpClicksPerSecond;
    setPumpToRawValue(pumpPct * PUMP_RANGE);
  }

  return pumpPct;
}
