#include <main.h>
#include "Heater.h"




#ifdef STM32_BOARD

ModbusRTUSlave modbus(USART_Modbus);
#else

SoftwareSerial mySerial(bt_rxPin,bt_txPin);

ModbusRTUSlave modbus(mySerial);

#endif

uint16_t holdingRegisters[5];
bool coils[2];
bool discreteInputs[2];
uint16_t inputRegisters[2];

Heater myHeater;


void TaskModbus( void *pvParameters );
void TaskAnalogRead( void *pvParameters );
void TaskTune( void *pvParameters );
 TaskHandle_t pidTaskHandle;

// the setup function runs once when you press reset or power the board
void setup() {


holdingRegisters[0]=34*10; //temperature setup



thermocoupleInit();

 modbus.configureHoldingRegisters(holdingRegisters,5);

// modbus.configureCoils(coils, 2);                       // bool array of coil values, number of coils
 //  modbus.configureDiscreteInputs(discreteInputs, 2);     // bool array of discrete input values, number of discrete inputs

  // modbus.configureInputRegisters(inputRegisters, 2);     // unsigned 16 bit integer array of input register values, number of input registers

#ifdef STM32_BOARD
   modbus.begin(1,9600);  //address 1, 9600, n81
#else
   modbus.begin(1,9600,SERIAL_8N1);  //address 1, 9600, n81
#endif


#ifdef STM32_BOARD


#else
//for uno debug
  //initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }


#endif




  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskModbus
    ,  "Modbus"   // A name just for humans
    ,  64  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskAnalogRead
    ,  "AnalogRead"
    ,  64  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL );

   xTaskCreate(
    TaskTune
    ,  "task tune"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );



#ifdef STM32_BOARD
  // start scheduler
  vTaskStartScheduler();
  #endif


  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}
static void sensorsReadTemperature(void) {


   // currentState.temperature = thermocoupleRead() - runningCfg.offsetTemp;  //changed by xhjiang
   holdingRegisters[1] = thermocoupleRead()*10 ;


  

 

}

static void sensorsRead(void){

  sensorsReadTemperature();
}
void loop()
{

}



  



/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskModbus(void *pvParameters)  // This is a task.
{
  (void) pvParameters;





  for (;;) // A Task shall never return or exit.
  {


      modbus.poll();
 
vTaskDelay(50 / portTICK_PERIOD_MS);  
  }
}



void TaskAnalogRead(void *pvParameters)  // This is a task.
{
  (void) pvParameters;


  for (;;)
  {
    
sensorsRead();



    vTaskDelay(500 / portTICK_PERIOD_MS);  
  }


}

void TaskTune(void *pvParameters)  // This is a task.
{
  (void) pvParameters;



pinMode(Pwm_Pin,OUTPUT);
#ifdef STM32_BOARD
  // start scheduler

#else

//Timer1.initialize(50000); //set the frequency in us

#endif


myHeater.gOutputPwr=0;

myHeater.tuning_on();



  for (;;)
  {
    



 //  double output=   myHeater.Compute(holdingRegisters[0]/10.0, holdingRegisters[1] /10);

#ifdef STM32_BOARD
  // 

#else



//Timer1.pwm(Pwm_Pin,output/100*1024);



  #endif

   myHeater.time_now=millis();

  myHeater.gTargetTemp=holdingRegisters[0]/10; 
  myHeater.gInputTemp=holdingRegisters[1]/10;


  myHeater.heat();
  myHeater.updateHeater();
  holdingRegisters[2]=myHeater.gOutputPwr*10;


    vTaskDelay(10 / portTICK_PERIOD_MS);  
  }
}

