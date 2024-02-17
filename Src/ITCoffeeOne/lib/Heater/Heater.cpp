#include "Heater.h"
#include <Arduino.h>
#include <FreeRTOSConfig.h>





    //Define Variables we'll be connecting to


//Specify the links and initial tuning parameters


    


Heater::Heater()
:myPID(&gInputTemp, &gOutputPwr, &gTargetTemp, gP, gI, gD, DIRECT)
{



myPID.SetTunings(gP, gI, gD);
myPID.SetSampleTime(PID_INTERVAL);
myPID.SetOutputLimits(0,100);


myPID.SetMode(AUTOMATIC);

time_now=millis();
time_last=time_now;

}

void Heater::initialize()
{

}




 void Heater:: SetOutputLimit(double min ,double max)
 {

  myPID.SetOutputLimits(min,max);
 }



 #define abs(x) ((x)>0?(x):-(x))





void Heater::tuning_on() {
  tune_time = 0;
  tune_start = 0;
  tune_count = 0;
  UpperCnt = 0; LowerCnt = 0;
  AvgUpperT = 0; AvgLowerT = 0;
  maxUpperT = 0; minLowerT = 0;
  myPID.SetMode(MANUAL);
  tuning = true;
}

void Heater::tuning_off() {
  myPID.SetMode(AUTOMATIC);
  tuning = false;

if(tune_count>0&&LowerCnt>0&&UpperCnt>0)
{
  double dt = float(tune_time - tune_start) / tune_count;


  double dT = ((AvgUpperT / UpperCnt) - (AvgLowerT / LowerCnt));

  double Ku = 4 * (2 * aTuneStep) / (dT * 3.14159);
  double Pu = dt / 1000; // units of seconds





  gP = 0.6 * Ku;
  gI = 1.2 * Ku / Pu;
  gD = 0.075 * Ku * Pu;

}



}
void Heater:: setHeatPowerPercentage(float power_in_100_percent) {
  if (power_in_100_percent < 0.0) {
    power_in_100_percent = 0.0;
  }
  if (power_in_100_percent > 100) {
    power_in_100_percent = 100;
  }
  heatcycles = power_in_100_percent*HEATER_INTERVAL/100;  
}


void Heater::loop()
{  
  time_now=millis();

   if (abs(time_now - time_last) >= PID_INTERVAL or time_last > time_now) {
   
    if (poweroffMode == true) {

      //power off
      gOutputPwr = 0;
      setHeatPowerPercentage(gOutputPwr);
    }
    else if (externalControlMode == true) {
      //external control
      gOutputPwr = 100 * gButtonState;
      setHeatPowerPercentage(gOutputPwr);
    }
    else 
     if (tuning == true){
        //tune
        tuning_loop();
    
    }
    else{

      //pid control
      // manual: late more than 10, or great 
           if ( !osmode && ( gInputTemp<(gTargetTemp- gOvershoot) |gInputTemp>gTargetTemp)) {
         // myPID.SetTunings(gaP, gaI, gaD);
           myPID.SetMode(MANUAL);

         

          osmode = true;
            }
            else if ( osmode &&   gInputTemp>= (gTargetTemp- gOvershoot) &&(gInputTemp<=gTargetTemp)) {
            //here is determine when exit overshoot mode

            
             gOutputPwr=0;
               myPID.SetMode(AUTOMATIC);
              myPID.SetTunings(gP, gI, gD);

              osmode = false;
            }


          if(osmode)
          {
            //manul control
        
              if(gInputTemp>gTargetTemp)gOutputPwr=0;
           else
           gOutputPwr=100;

            setHeatPowerPercentage(gOutputPwr);
          }
          else
          {
              if (myPID.Compute() == true) {
                setHeatPowerPercentage(gOutputPwr);
              }
          }

         }



    time_last=time_now;
   
    }

       updateHeater();  //update every loop, so , out of interval.

}

void Heater::updateHeater() {
  boolean h;
  heatCurrentTime = time_now;
  if (heatCurrentTime - heatLastTime >= HEATER_INTERVAL or heatLastTime > heatCurrentTime) { //second statement prevents overflow errors
    // begin cycle
    Serial.println("1");
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





void Heater::tuning_loop() {
  
 
  // count seconds between power-on-cycles
  //
  //
  if (gInputTemp <= (gTargetTemp - aTuneThres)) { // below lower threshold -> power on
    if (gOutputPwr == 0) { // just fell below threshold
      if (tune_count == 0) tune_start = time_now;
      tune_time = time_now;
      tune_count++;
      //AvgLowerT += gInputTemp;
      //LowerCnt++;
    }
    if (minLowerT > gInputTemp || minLowerT == 0) minLowerT = gInputTemp;
    gOutputPwr = aTuneStep;
    setHeatPowerPercentage(aTuneStep);


    // Serial.println(gOutputPwr);

  }
  else if (gInputTemp >= (gTargetTemp + aTuneThres)) { // above upper threshold -> power off
    //if (gOutputPwr == aTuneStep) { // just crossed upper threshold
      //AvgUpperT += gInputTemp;
      //UpperCnt++;
    //}
    if (maxUpperT < gInputTemp) maxUpperT = gInputTemp;
    gOutputPwr = 0;
    setHeatPowerPercentage(0);

    // Serial.println("0");
  }

  // store min / max
  if ((gInputTemp > (gTargetTemp - (aTuneThres / 2))) && (gInputTemp < (gTargetTemp + (aTuneThres / 2)))) {
    if (maxUpperT != 0) {
      Serial.println("Adding new upper T");
      Serial.println(maxUpperT);
      AvgUpperT += maxUpperT;
      UpperCnt ++;
      maxUpperT = 0;
    }
    if (minLowerT != 0) {
      Serial.println("Adding new lower T");
      Serial.println(minLowerT);
      AvgLowerT += minLowerT;
      LowerCnt ++;
      minLowerT = 0;
    }
  }
}



 
 

