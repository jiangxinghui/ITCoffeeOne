#ifndef Heater_Header
#define Heater_header


//#include <PID_v1.h>
#include <Arduino.h>


  
class Heater {
public:






#define HEATER_INTERVAL 1000


double gOutputPwr = 0.0;
int Pwr_Out_Low_Percent=10;  //0-100




float heatcycles; // the number of millis out of 1000 for the current heat amount (percent * 10)

 bool heaterState = 0;

 //for gag
    uint32_t heaterWave;



unsigned long heatCurrentTime = 0, heatLastTime = 0;

unsigned long brewOffTime=0;




bool tuning = false;
bool osmode = false;
bool poweroffMode = false;
bool externalControlMode = false;






//char Status[256];

  Heater(int hpwr);

  void initialize();




void SetOutputLimit(double min ,double max);


void loop();

void justDoCoffee(float targetTemperature,float temperature, const bool brewActive,float pumpFlow);


private:
unsigned long time_now;
//
int hpwr;
int mainDivider;
int brewDivider;
float maxHeatTemperature;


void updateHeater();


void turnHeatElementOnOff(bool on) ;
void setBoilerOff(void) ;
void setBoilerOn(void) ;

 void setHeatPowerPercentage(float power);

void pulseHeaters(const uint32_t pulseLength, const int factor_1, const int factor_2, const bool brewActive);


};


#endif
