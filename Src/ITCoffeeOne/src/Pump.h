#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>




void pump_UpdateZC();


void pumpInit(unsigned char sensePin, unsigned char controlPin,const int powerLineFrequency, 
const float pumpFlowAtZero);
float setPumpPressure(const float targetPressure, const float flowRestriction,
 float currentPressure,float currentFlow,float currentPressureChangeSpeed) ;
void setPumpOff(void);

void setPumpFullOn(void);
void setPumpToRawValue(const uint8_t val);
long  getAndResetClickCounter(void);
int getCPS(void);
void pumpPhaseShift(void);
void pumpStopAfter(const uint8_t val);
float getPumpFlow(const float cps, const float pressure);
float getPumpFlowPerClick(const float pressure);
float getClicksPerSecondForFlow(const float flow, const float pressure);
float setPumpFlow(const float targetFlow, const float pressureRestriction, float currentPressure,float currentFlow,float currentPressureChangeSpeed);
float getPumpPct(const float targetPressure, const float flowRestriction, float currentPressure,float currentFlow,float currentPressureChangeSpeed);












#endif
