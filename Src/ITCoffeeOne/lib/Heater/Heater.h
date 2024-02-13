#ifndef Heater_Header
#define Heater_header


#include <PID_v1.h>



  
class Heater {
public:



  #define S_P 115.0
#define S_I 4.0
#define S_D 850.0
#define S_aP 100.0
#define S_aI 0.0
#define S_aD 0.0
#define S_TSET 35.0
#define S_TBAND 1.5

#define PID_INTERVAL 200
#define HEATER_INTERVAL 1000

double gTargetTemp = S_TSET;
double gOvershoot = S_TBAND;
double gInputTemp = 20.0;
double gOutputPwr = 0.0;
double gP = S_P, gI = S_I, gD = S_D;
double gaP = S_aP, gaI = S_aI, gaD = S_aD;


double aTuneStep = 100.0, aTuneThres = 0.2;
double maxUpperT = 0, minLowerT = 0;
double AvgUpperT = 0, AvgLowerT = 0;
int UpperCnt = 0, LowerCnt = 0;
int tune_count = 0;

unsigned long tune_time = 0;
unsigned long tune_start = 0;

unsigned long time_now = 0;
unsigned long time_last = 0;

float heatcycles; // the number of millis out of 1000 for the current heat amount (percent * 10)

bool heaterState = 0;
unsigned long heatCurrentTime = 0, heatLastTime = 0;



int gButtonState = 0;


bool tuning = false;
bool osmode = false;
bool poweroffMode = false;
bool externalControlMode = false;


//char Status[256];

  Heater();

  void initialize();

  double Compute(double setpoint, double input);


void SetOutputLimit(double min ,double max);

void tuning_on();
void tuning_off();
void heat();
void tune();
void updateHeater();

void setHeatPowerPercentage(float power);
void  statusAsJson() ;
private:
PID myPID;





void turnHeatElementOnOff(bool on) ;
 
  void adaptTunings();
};


#endif
