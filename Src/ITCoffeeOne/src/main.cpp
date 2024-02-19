#include <main.h>
#include "Heater.h"
#include "peripherals/peripherals.h"



#ifdef STM32_BOARD

ModbusRTUSlave modbus(USART_Modbus);
#else


SoftwareSerial mySerial(bt_rxPin,bt_txPin);

ModbusRTUSlave modbus(mySerial);

#endif

uint16_t holdingRegisters[10];
bool coils[2];
SensorState currentState;

Heater myHeater;
 double temperature;




// the setup function runs once when you press reset or power the board
void setup() {

#ifdef STM32_BOARD

#else
debug_init();

#endif



pinMode(HeaterPin,OUTPUT);
myHeater.gOutputPwr=0;

pinMode(LED_BUILTIN,OUTPUT);

coils[0]=false;
coils[1]=false;

holdingRegisters[0]=S_TSET*10;
thermocoupleInit();

 modbus.configureHoldingRegisters(holdingRegisters,10);

 modbus.configureCoils(coils, 2);                       // bool array of coil values, number of coils
 //  modbus.configureDiscreteInputs(discreteInputs, 2);     // bool array of discrete input values, number of discrete inputs

  // modbus.configureInputRegisters(inputRegisters, 2);     // unsigned 16 bit integer array of input register values, number of input registers

#ifdef STM32_BOARD
   modbus.begin(1,9600);  //address 1, 9600, n81
#else
   modbus.begin(1,9600,SERIAL_8N1);  //address 1, 9600, n81
#endif









  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}
static void sensorsReadTemperature(void) {

 if (millis() > thermoTimer) {
   // currentState.temperature = thermocoupleRead() - runningCfg.offsetTemp;  //changed by xhjiang
 
 temperature= thermocoupleRead() ;

  thermoTimer = millis() + GET_KTYPE_READ_EVERY;


 }

}
static void sensorReadSwitches(void) {
  currentState.brewSwitchState = brewState();
 // currentState.steamSwitchState = steamState();
 // currentState.hotWaterSwitchState = waterPinState() || (currentState.brewSwitchState && currentState.steamSwitchState); // use either an actual switch, or the GC/GCP switch combo
}

static void sensorsRead(void){

sensorReadSwitches();

 sensorsReadTemperature();
}

static void heartBeat(void)
{
  uint32_t elapsedTime=millis()-HeartBeatTimer;
  if(elapsedTime>500)
  {
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));

  HeartBeatTimer=millis();



  }


}



void loop()
{
  heartBeat();

 sensorsRead();

   holdingRegisters[1]=temperature*10;


  holdingRegisters[2]=myHeater.gOutputPwr*10;
holdingRegisters[3]=myHeater.gP*100;
holdingRegisters[4]=myHeater.gD*100;
holdingRegisters[5]=myHeater.gI*100;
holdingRegisters[6]=myHeater.tuning;
holdingRegisters[7]=myHeater.tune_count;


holdingRegisters[8]=myHeater.osmode;
holdingRegisters[9]=brewState();


      modbus.poll();
myHeater.justDoCoffee(holdingRegisters[0]/10, temperature,currentState.brewSwitchState);


  // myHeater.gTargetTemp=(double)holdingRegisters[0]/10; 
  // myHeater.gInputTemp=(double)holdingRegisters[1]/10;


  // myHeater.loop();

   digitalWrite(HeaterPin,myHeater.heaterState);


    if(coils[0]==true)
    {myHeater.tuning_on();
    coils[0]=false;

   // Serial.println("start tune");

    }

   if(coils[1]==true)
    {myHeater.tuning_off();

   // Serial.println("stop tune");

    coils[1]=false;

    }
}



  







