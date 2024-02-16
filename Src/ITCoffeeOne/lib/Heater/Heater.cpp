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

time_now=millis();
time_last=time_now;

}

void Heater::initialize()
{

}


double Heater::Compute(double setpoint,double input)
{

    gInputTemp=input;

    gTargetTemp=setpoint;

   // adaptTunings();
// if(tuning==true)
// {
//     tune();
// }
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
Serial.print("tune time");
Serial.print(tune_time);
Serial.print("tune start");
Serial.print(tune_start);
Serial.print("tune count");
Serial.print(tune_count);
Serial.println();
if(tune_count>0&&LowerCnt>0&&UpperCnt>0)
{
  double dt = float(tune_time - tune_start) / tune_count;

Serial.print("avg upper");
Serial.print(AvgUpperT);
Serial.print("uper count");
Serial.print(UpperCnt);
Serial.print("average lower");
Serial.print(AvgLowerT);
Serial.print("lower count");
Serial.print(LowerCnt);

Serial.println();
  double dT = ((AvgUpperT / UpperCnt) - (AvgLowerT / LowerCnt));

  double Ku = 4 * (2 * aTuneStep) / (dT * 3.14159);
  double Pu = dt / 1000; // units of seconds


Serial.print("ku");
Serial.print(Ku);
Serial.print("Pu");
Serial.print(Pu);
Serial.println();


  gP = 0.6 * Ku;
  gI = 1.2 * Ku / Pu;
  gD = 0.075 * Ku * Pu;

}


  Serial.print("gp");
  Serial.print(gP );
    Serial.print("gi");
  Serial.print(gI );
    Serial.print("gd");
  Serial.print(gD );
Serial.println();
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
      gOutputPwr = 1000 * gButtonState;
      setHeatPowerPercentage(gOutputPwr);
    }
    else 
     if (tuning == true){
        //tune
        tuning_loop();
    
    }
    else{

      //pid control
           if ( !osmode && abs(gTargetTemp - gInputTemp) >= gOvershoot ) {
          myPID.SetTunings(gaP, gaI, gaD);
          osmode = true;
            }
            else if ( osmode && abs(gTargetTemp - gInputTemp) < gOvershoot ) {
              myPID.SetTunings(gP, gI, gD);
              osmode = false;
            }


            if (myPID.Compute() == true) {
              setHeatPowerPercentage(gOutputPwr);
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



 
 

