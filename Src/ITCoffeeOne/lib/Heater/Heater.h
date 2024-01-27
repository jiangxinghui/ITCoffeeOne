#ifndef Heater_Header
#define Heater_header


#include <PID_v1.h>

  

  
class Heater {
public:

double Kp =100, Ki=5 , Kd=1 ;
  double Setpoint = 83, Input = 0;
  double Output = 10;
  Heater();

  void initialize();

  double Compute(double setpoint, double input);


void SetOutputLimit(double min ,double max);


private:
PID myPID;


 
  void adaptTunings();
};


#endif
