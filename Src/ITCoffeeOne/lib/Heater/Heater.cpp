#include "Heater.h"
#include <Arduino.h>


#include "utils.h"



    //Define Variables we'll be connecting to


//Specify the links and initial tuning parameters


    


Heater::Heater(int hpwr)
//:myPID(&gInputTemp, &gOutputPwr, &gTargetTemp, gP, gI, gD, DIRECT)
{

this->hpwr=hpwr;

mainDivider=5;
brewDivider=3;
maxHeatTemperature=120;

// myPID.SetTunings(gP, gI, gD);
// myPID.SetSampleTime(PID_INTERVAL);
// myPID.SetOutputLimits(0,100);


// myPID.SetMode(AUTOMATIC);

time_now=millis();
time_last=time_now;

}

void Heater::initialize()
{

}




 void Heater:: SetOutputLimit(double min ,double max)
 {

 // myPID.SetOutputLimits(min,max);
 }



 #define abs(x) ((x)>0?(x):-(x))





 void Heater::tuning_on() {
//   tune_time = 0;
//   tune_start = 0;
//   tune_count = 0;
//   UpperCnt = 0; LowerCnt = 0;
//   AvgUpperT = 0; AvgLowerT = 0;
//   maxUpperT = 0; minLowerT = 0;
//   myPID.SetMode(MANUAL);
//   tuning = true;
 }

 void Heater::tuning_off() {
//   myPID.SetMode(AUTOMATIC);
//   tuning = false;

// if(tune_count>0&&LowerCnt>0&&UpperCnt>0)
// {
//   double dt = float(tune_time - tune_start) / tune_count;


//   double dT = ((AvgUpperT / UpperCnt) - (AvgLowerT / LowerCnt));

//   double Ku = 4 * (2 * aTuneStep) / (dT * 3.14159);
//   double Pu = dt / 1000; // units of seconds





//   gP = 0.6 * Ku;
//   gI = 1.2 * Ku / Pu;
//   gD = 0.075 * Ku * Pu;

}



// }
void Heater:: setHeatPowerPercentage(float power_in_100_percent) {
  if (power_in_100_percent < 0.0) {
    power_in_100_percent = 0.0;
  }
  if (power_in_100_percent > 100) {
    power_in_100_percent = 100;
  }
  heatcycles = power_in_100_percent*HEATER_INTERVAL/100;  
}


// void Heater::loop()
// {  
//   time_now=millis();

//    if (abs(time_now - time_last) >= PID_INTERVAL or time_last > time_now) {
   
//     if (poweroffMode == true) {

//       //power off
//       gOutputPwr = 0;
//       setHeatPowerPercentage(gOutputPwr);
//     }
//     // else if (externalControlMode == true) {
//     //   //external control
//     //   gOutputPwr = 100 * gButtonState;
//     //   setHeatPowerPercentage(gOutputPwr);
//     // }
//     else 
//      if (tuning == true){
//         //tune
//         tuning_loop();
    
//     }
//     else{

//       //pid control
//       // manual: late more than 10, or great 
//         //    if ( !osmode && ( gInputTemp<(gTargetTemp- gOvershoot) |gInputTemp>gTargetTemp)) {
//         //  // myPID.SetTunings(gaP, gaI, gaD);
//         //    myPID.SetMode(MANUAL);

         

//         //   osmode = true;
//         //     }
//         //     else if ( osmode &&   gInputTemp>= (gTargetTemp- gOvershoot) &&(gInputTemp<=gTargetTemp)) {
//         //     //here is determine when exit overshoot mode

            
//         //      gOutputPwr=0;
//         //        myPID.SetMode(AUTOMATIC);
//         //       myPID.SetTunings(gP, gI, gD);

//         //       osmode = false;
//         //     }


//         //   if(osmode)
//         //   {
//         //     //manul control
        
//         //       if(gInputTemp>gTargetTemp)gOutputPwr=0;
//         //    else
//         //    gOutputPwr=100;

//         //     setHeatPowerPercentage(gOutputPwr);
//         //   }
//         //   else
//         //   {
//         //       if (myPID.Compute() == true) {
//         //         setHeatPowerPercentage(gOutputPwr);
//         //       }
//         //   }

//         //  
//         }



//     time_last=time_now;
   
//     }

//        updateHeater();  //update every loop, so , out of interval.

// }

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


// void Heater::tuning_loop() {
  
 
//   // count seconds between power-on-cycles
//   //
//   //
//   if (gInputTemp <= (gTargetTemp - aTuneThres)) { // below lower threshold -> power on
//     if (gOutputPwr == 0) { // just fell below threshold
//       if (tune_count == 0) tune_start = time_now;
//       tune_time = time_now;
//       tune_count++;
//       //AvgLowerT += gInputTemp;
//       //LowerCnt++;
//     }
//     if (minLowerT > gInputTemp || minLowerT == 0) minLowerT = gInputTemp;
//     gOutputPwr = aTuneStep;
//     setHeatPowerPercentage(aTuneStep);


//     // Serial.println(gOutputPwr);

//   }
//   else if (gInputTemp >= (gTargetTemp + aTuneThres)) { // above upper threshold -> power off
//     //if (gOutputPwr == aTuneStep) { // just crossed upper threshold
//       //AvgUpperT += gInputTemp;
//       //UpperCnt++;
//     //}
//     if (maxUpperT < gInputTemp) maxUpperT = gInputTemp;
//     gOutputPwr = 0;
//     setHeatPowerPercentage(0);

//     // Serial.println("0");
//   }

//   // store min / max
//   if ((gInputTemp > (gTargetTemp - (aTuneThres / 2))) && (gInputTemp < (gTargetTemp + (aTuneThres / 2)))) {
//     if (maxUpperT != 0) {
//      // Serial.println("Adding new upper T");
//     //  Serial.println(maxUpperT);
//       AvgUpperT += maxUpperT;
//       UpperCnt ++;
//       maxUpperT = 0;
//     }
//     if (minLowerT != 0) {
//     //  Serial.println("Adding new lower T");
//      // Serial.println(minLowerT);
//       AvgLowerT += minLowerT;
//       LowerCnt ++;
//       minLowerT = 0;
//     }
//   }
// }



void Heater::pulseHeaters(const uint32_t pulseLength, const int factor_1, const int factor_2, const bool brewActive) {
  static bool heaterState2;
   uint32_t interval;
  
 
  if (!heaterState2 && ((millis() - heaterWave) > (pulseLength * factor_1))) {
    brewActive ? turnHeatElementOnOff(0) : turnHeatElementOnOff(1);
        
    heaterState2=!heaterState2;
  interval=pulseLength * factor_1;

    heaterWave=millis();
  } else if (heaterState2 && ((millis() - heaterWave) > (pulseLength / factor_2))) {
    brewActive ? turnHeatElementOnOff(1) : turnHeatElementOnOff(0);
       
    heaterState2=!heaterState2;
      interval=pulseLength / factor_2;
    heaterWave=millis();
  }

  
}



void Heater::justDoCoffee(float targetTemperature,float temperature, const bool brewActive) {

  //float brewTempSetPoint = ACTIVE_PROFILE(runningCfg).setpoint + runningCfg.offsetTemp;
   float brewTempSetPoint = targetTemperature;
  float sensorTemperature = temperature;// + runningCfg.offsetTemp;


 

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
        //float tempDelta = TEMP_DELTA(brewTempSetPoint, currentState);
          float tempDelta = brewTempSetPoint/10;

        float BREW_TEMP_DELTA = mapRange(sensorTemperature, brewTempSetPoint, brewTempSetPoint + tempDelta, tempDelta, 0, 0);
        deltaOffset = constrain(BREW_TEMP_DELTA, 0, tempDelta);
      }
      if (sensorTemperature <= brewTempSetPoint + deltaOffset) {

        // pulseHeaters(runningCfg.hpwr, runningCfg.mainDivider, runningCfg.brewDivider, brewActive);
        gOutputPwr=(1/(double)brewDivider)/(mainDivider+1/(double)brewDivider)*100;

         pulseHeaters(hpwr, mainDivider, brewDivider, brewActive);
       
      } else {
        gOutputPwr=0;
        
            turnHeatElementOnOff(0);
      }
    }
  } else { //if brewState == false
    if (sensorTemperature <= ((float)brewTempSetPoint - 30.f)) {
         turnHeatElementOnOff(1);
      gOutputPwr=100; //added by itcoffee
    } else {
      // int HPWR_LOW = runningCfg.hpwr / runningCfg.mainDivider;
      int HPWR_LOW = hpwr / mainDivider;

      // Calculating the boiler heating power range based on the below input values
      //int HPWR_OUT = mapRange(sensorTemperature, brewTempSetPoint - 30, brewTempSetPoint, runningCfg.hpwr, HPWR_LOW, 0);
      int HPWR_OUT = mapRange(sensorTemperature, brewTempSetPoint - 30, brewTempSetPoint, hpwr, HPWR_LOW, 0);

     // HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, runningCfg.hpwr);  // limits range of sensor values to HPWR_LOW and HPWR
      HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, hpwr);  // limits range of sensor values to HPWR_LOW and HPWR

 if (sensorTemperature <= ((float)brewTempSetPoint - 20.f)) {
      //  pulseHeaters(HPWR_OUT, 1, runningCfg.mainDivider, brewActive);
      
   
   
       pulseHeaters(HPWR_OUT, 1, mainDivider/2, brewActive);
        gOutputPwr=(1/(double)mainDivider*2)/(1+1/(double)mainDivider*2)*100;


 
      }
      else   if (sensorTemperature <= ((float)brewTempSetPoint - 10.f)) {
      //  pulseHeaters(HPWR_OUT, 1, runningCfg.mainDivider, brewActive);
      
   
   
       pulseHeaters(HPWR_OUT, 1, mainDivider, brewActive);
        gOutputPwr=(1/(double)mainDivider)/(1+1/(double)mainDivider)*100;


 
      } else if (sensorTemperature < ((float)brewTempSetPoint)) {
       // pulseHeaters(HPWR_OUT,  runningCfg.brewDivider, runningCfg.brewDivider, brewActive);
       
          
      
         pulseHeaters(HPWR_OUT,  brewDivider, brewDivider/2, brewActive);
        gOutputPwr=(1/(double)brewDivider*2)/(brewDivider+1/(double)brewDivider*2)*100;

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
//  //test only
// setHeatPowerPercentage(gOutputPwr);//
// updateHeater( );  //use our heater cycle

  
}





 
 

