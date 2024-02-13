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
myPID.SetOutputLimits(0,1000);


myPID.SetMode(AUTOMATIC);



}

void Heater::initialize()
{

}


double Heater::Compute(double setpoint,double input)
{

    gInputTemp=input;

    gTargetTemp=setpoint;

   // adaptTunings();
if(tuning==true)
{
    tune();
}
//myPID.Compute();
  





// Serial.println(" in:"+String(Input)+",out:"+String(Output));
 //return Heater::Output;
}

 void Heater:: SetOutputLimit(double min ,double max)
 {

  myPID.SetOutputLimits(min,max);
 }



 #define abs(x) ((x)>0?(x):-(x))

void Heater::adaptTunings() {
  //double gap = Setpoint - Input;

  // // 根据距离设定值的距离选择PID参数
  //  if (gap > 5 ) {
  //   myPID.SetTunings(5, 5, 1); // 使用激进的PID参数
  //   Serial.println("fast");
  // } else 
  // if (gap> 3 ) {
    
  //   myPID.SetTunings(3, 1, 2); // 使用标准的PID参数
  //   Serial.println("normal");
  // } else if(gap>0) {
  //   myPID.SetTunings(0.5, 0.5, 2.0); // 使用保守的PID参数
  //    Serial.println("slow");
  // }
  // else
  // { myPID.SetTunings(10, 10, 2.0); // 使用保守的PID参数
  //    Serial.println("man=0");

  // }

    if ( !osmode && abs(gTargetTemp - gInputTemp) >= 10 ) {
        myPID.SetTunings(gaP, gaI, gaD);
        osmode = true;
      }
      else if ( osmode && abs(gTargetTemp - gInputTemp) < 10 ) {
        myPID.SetTunings(gP, gI, gD);
        osmode = false;
      }


}


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

  double dt = float(tune_time - tune_start) / tune_count;
  double dT = ((AvgUpperT / UpperCnt) - (AvgLowerT / LowerCnt));

  double Ku = 4 * (2 * aTuneStep) / (dT * 3.14159);
  double Pu = dt / 1000; // units of seconds

  gP = 0.6 * Ku;
  gI = 1.2 * Ku / Pu;
  gD = 0.075 * Ku * Pu;
}
void Heater:: setHeatPowerPercentage(float power) {
  if (power < 0.0) {
    power = 0.0;
  }
  if (power > 1000.0) {
    power = 1000.0;
  }
  heatcycles = power;
}


void Heater::heat()
{  
   if (abs(time_now - time_last) >= PID_INTERVAL or time_last > time_now) {
   
   
   if (tuning == true)
    {

      tune();
    }
    time_last=time_now;
   
    }
}

void Heater::updateHeater() {
  boolean h;
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
  digitalWrite(9, on); //turn pin high
  heaterState = on;
}





void Heater::tune() {
  
 
  // count seconds between power-on-cycles
  //
  //
  if (gInputTemp < (gTargetTemp - aTuneThres)) { // below lower threshold -> power on
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
  else if (gInputTemp > (gTargetTemp + aTuneThres)) { // above upper threshold -> power off
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


void Heater::statusAsJson() {
  
//  strcpy(Status,"time");

// char myString[20];  // Choose a size that can accommodate the double and additional characters

// ultoa(time_now, myString, 10);  // 10 is the base (decimal in this case)


//    strcat(Status,myString);
//    strcat(Status,"mesauredTemperature");

  
// dtostrf(gInputTemp, 6, 2, myString);  // 6 is the total width, 2 is the number of decimal places

//     strcat(Status, myString);
//    strcat(Status,"targetTemperature");
//    dtostrf(gTargetTemp, 6, 2, myString);  // 6 is the total width, 2 is the number of decimal places

//    strcat(Status,myString);
//    strcat(Status,"heaterPower");
//    dtostrf(gOutputPwr, 6, 2, myString);  // 6 is the total width, 2 is the number of decimal places

//    strcat(Status,myString );
  

 
 
}
