#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>
#define PUMP_INTERVAL 1000


class  Pump
{
private:
    /* data */

     uint8_t PUMP_RANGE = 100;


float flowPerClickAtZeroBar = 0.27f;
int maxPumpClicksPerSecond = 50;
float fpc_multiplier = 1.2f;
bool zcup=false;
unsigned long heatCurrentTime = 0, heatLastTime = 0;

unsigned long time_now;
#define HEATER_INTERVAL 1000


float pumpcycles; // the number of millis out of 1000 for the current pump (percent * 10)

  volatile long _counter;

  
float pressureInefficiencyCoefficient[7] ={
  0.045f,
  0.015f,
  0.0033f,
  0.000685f,
  0.000045f,
  0.009f,
  -0.0018f
};






public:
   Pump();

    bool pumpPinState=0;

void pump_UpdateZC();


void pumpInit( 
const float pumpFlowAtZero);
float setPumpPressure(const float targetPressure, const float flowRestriction,
 float currentPressure,float currentFlow,float currentPressureChangeSpeed) ;

float getPumpFlow(const float cps, const float pressure);
float getPumpFlowPerClick(const float pressure);
float getClicksPerSecondForFlow(const float flow, const float pressure);
float setPumpFlow(const float targetFlow, const float pressureRestriction, float currentPressure,float currentFlow,float currentPressureChangeSpeed);
float getPumpPct(const float targetPressure, const float flowRestriction, float currentPressure,float currentFlow,float currentPressureChangeSpeed);

void setPumpOff();

 void setPumpPowerPercentage(float power);

 long getAndResetClickCounter();

void updatePump() ;

void  turnPumpElementOnOff(bool on) ;



};







#endif
