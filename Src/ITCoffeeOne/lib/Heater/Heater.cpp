#include "Heater.h"
#include <Arduino.h>


#include "utils.h"



    //Define Variables we'll be connecting to


//Specify the links and initial tuning parameters


    


Heater::Heater(int hpwr)

{

this->hpwr=hpwr;

mainDivider=5;
brewDivider=3;
maxHeatTemperature=120;




}

void Heater::initialize()
{

}




 void Heater:: SetOutputLimit(double min ,double max)
 {

 // myPID.SetOutputLimits(min,max);
 }



 #define abs(x) ((x)>0?(x):-(x))








void Heater:: setHeatPowerPercentage(float power_in_100_percent) {
  if (power_in_100_percent < 0.0) {
    power_in_100_percent = 0.0;
  }
  if (power_in_100_percent > 100) {
    power_in_100_percent = 100;
  }
  heatcycles = power_in_100_percent*HEATER_INTERVAL/100;  
}




void Heater::updateHeater() {
   time_now=millis();
  heatCurrentTime = time_now;

   if (heatCurrentTime - heatLastTime >= HEATER_INTERVAL or heatLastTime > heatCurrentTime) { //second statement prevents overflow errors
  
    // begin cycle
  
    turnHeatElementOnOff(1);  //
    heatLastTime = heatCurrentTime;
  }
  if (heatCurrentTime - heatLastTime >= heatcycles) {
  

    
    turnHeatElementOnOff(0);
  }

  
    
}
void Heater::turnHeatElementOnOff(bool on) {
  //digitalWrite(HeaterPin, on); //turn pin high , change to main.cpp
  heaterState = on;
  
}

inline static float TEMP_DELTA(float d, float pumpFlow) {
  return (
    d * (pumpFlow < 1.f
      ? pumpFlow / 7.f
      : pumpFlow / 5.f
    )
  );
}





void Heater::justDoCoffee(float targetTemperature,float temperature, const bool brewActive,float pumpFlow) {

  //float brewTempSetPoint = ACTIVE_PROFILE(runningCfg).setpoint + runningCfg.offsetTemp;
   float brewTempSetPoint = targetTemperature;
  float sensorTemperature = temperature;// + runningCfg.offsetTemp;
 int HPWR_OUT;

 

  if (brewActive) { //if brewState == true
    //all power beyond 5degree when brew
    if(sensorTemperature <= brewTempSetPoint - 5.f) {
     
     gOutputPwr=100; //added by itcoffee

      turnHeatElementOnOff(1);
    } else 
    
    {
      //inside 5 degree when brew:
      float deltaOffset = 0.f;
     // if (runningCfg.brewDeltaState) 
      {
        float tempDelta = TEMP_DELTA(brewTempSetPoint, pumpFlow);
      

        float BREW_TEMP_DELTA = mapRange(sensorTemperature, brewTempSetPoint, brewTempSetPoint + tempDelta, tempDelta, 0, 0);
        deltaOffset = constrain(BREW_TEMP_DELTA, 0, tempDelta);
      }
      if (sensorTemperature <= brewTempSetPoint + deltaOffset) {

        // pulseHeaters(runningCfg.hpwr, runningCfg.mainDivider, runningCfg.brewDivider, brewActive);
        gOutputPwr=0.5*100;

      
       
      } else {
        gOutputPwr=0;
        
            turnHeatElementOnOff(0);
      }
    }
  } else { //if brewState == false
    if (sensorTemperature <= ((float)brewTempSetPoint - 30.f)) {
         turnHeatElementOnOff(1);
      gOutputPwr=100; //added by itcoffee
    } else 
    
    {
if (sensorTemperature <= ((float)brewTempSetPoint - 20.f)) {
 
        gOutputPwr=50;


 
      }
      else   if (sensorTemperature <= ((float)brewTempSetPoint - 10.f)) {
      
   
      
     
             gOutputPwr=25;

 
      } 
   
      
      else if (sensorTemperature < ((float)brewTempSetPoint)) {
     
      
       
               gOutputPwr=10;


      } else {
         gOutputPwr=0; //added by itcoffee
            turnHeatElementOnOff(0);
      }
    }
  }
  // if (brewActive || !currentState.brewSwitchState) { // keep steam boiler supply valve open while steaming/descale only
  //   setSteamValveRelayOff();
  // }
  // setSteamBoilerRelayOff();
 
 if(temperature>maxHeatTemperature)
  {
    gOutputPwr=0;
    turnHeatElementOnOff(0);
  
  }
//  
 setHeatPowerPercentage(gOutputPwr);//
 updateHeater( );  //use our heater cycle

  
}





 
 

