#include "Heater.h"
#include <Arduino.h>
#include <FreeRTOSConfig.h>





    //Define Variables we'll be connecting to


//Specify the links and initial tuning parameters


    


Heater::Heater()
:myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT)
{




myPID.SetMode(AUTOMATIC);

myPID.SetOutputLimits(0,100);
myPID.SetSampleTime(200);




}

void Heater::initialize()
{

}


double Heater::Compute(double s,double i)
{

    Setpoint=s;

    Input=i;

    adaptTunings();

myPID.Compute();
  





// Serial.println(" in:"+String(Input)+",out:"+String(Output));
 return Heater::Output;
}

 void Heater:: SetOutputLimit(double min ,double max)
 {

  myPID.SetOutputLimits(min,max);
 }




void Heater::adaptTunings() {
  double gap = abs(Setpoint - Input);

  // 根据距离设定值的距离选择PID参数
  if (gap < 10) {
    myPID.SetTunings(50.0, 1.0, 0.0); // 使用保守的PID参数
  } else {
    myPID.SetTunings(115.0, 4.0, 10.0); // 使用激进的PID参数
  }
}


