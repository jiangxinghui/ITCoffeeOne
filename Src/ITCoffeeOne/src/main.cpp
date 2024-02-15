#include <main.h>
#include "Heater.h"




#ifdef STM32_BOARD

ModbusRTUSlave modbus(USART_Modbus);
#else

SoftwareSerial mySerial(bt_rxPin,bt_txPin);

ModbusRTUSlave modbus(mySerial);

#endif

uint16_t holdingRegisters[10];
bool coils[2];
bool discreteInputs[2];
uint16_t inputRegisters[2];

Heater myHeater;
double temperature;


void TaskModbus( void *pvParameters );
void TaskAnalogRead( void *pvParameters );
void TaskHeater( void *pvParameters );
 TaskHandle_t pidTaskHandle;

// the setup function runs once when you press reset or power the board
void setup() {


coils[0]=false;
coils[1]=false;


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
    TaskHeater
    ,  "heater task "
    ,  64  // Stack size
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
 
   temperature = thermocoupleRead() ;


  

 

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


holdingRegisters[0]=93*10;


  for (;;) // A Task shall never return or exit.
  {

#ifdef Simulate_Input
//holding register[1] is controlled external
#else

  holdingRegisters[1]=temperature*10;
#endif

  holdingRegisters[2]=myHeater.gOutputPwr*10;
holdingRegisters[3]=myHeater.gP*100;
holdingRegisters[4]=myHeater.gD*100;
holdingRegisters[5]=myHeater.gI*100;
holdingRegisters[6]=myHeater.tuning;
holdingRegisters[7]=myHeater.tune_count;
holdingRegisters[8]=myHeater.LowerCnt;
holdingRegisters[9]=myHeater.UpperCnt;

      modbus.poll();


    if(coils[0]==true)
    {myHeater.tuning_on();
    coils[0]=false;

    Serial.println("start tune");
    }

   if(coils[1]==true)
    {myHeater.tuning_off();

    Serial.println("stop tune");
  


    coils[1]=false;
   }


vTaskDelay(200 / portTICK_PERIOD_MS);  
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

void TaskHeater(void *pvParameters)  // This is a task.
{
  (void) pvParameters;



pinMode(HeaterPin,OUTPUT);
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

   

  myHeater.gTargetTemp=holdingRegisters[0]/10; 
  myHeater.gInputTemp=holdingRegisters[1]/10;


  myHeater.loop();
 
  digitalWrite(HeaterPin,myHeater.heaterState);


    vTaskDelay(10 / portTICK_PERIOD_MS);  
  }
}


