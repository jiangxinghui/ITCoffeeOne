#include <main.h>


//#include <avr8-stub.h>



SoftwareSerial mySerial(rxPin,txPin);

ModbusRTUSlave modbus(mySerial);

uint16_t holdingRegisters[5];
bool coils[2];
bool discreteInputs[2];
uint16_t inputRegisters[2];



void TaskModbus( void *pvParameters );
void TaskAnalogRead( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {


//debug_init();

thermocoupleInit();

 modbus.configureHoldingRegisters(holdingRegisters,5);

 modbus.configureCoils(coils, 2);                       // bool array of coil values, number of coils
   modbus.configureDiscreteInputs(discreteInputs, 2);     // bool array of discrete input values, number of discrete inputs

   modbus.configureInputRegisters(inputRegisters, 2);     // unsigned 16 bit integer array of input register values, number of input registers

   modbus.begin(1,9600,SERIAL_8N1);

  // initialize serial communication at 9600 bits per second:
  Serial.begin(19200);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }




  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskModbus
    ,  "Modbus"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskAnalogRead
    ,  "AnalogRead"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
//一般不放代码这里。 优先级最低

}

static void sensorsReadTemperature(void) {


   // currentState.temperature = thermocoupleRead() - runningCfg.offsetTemp;  //changed by xhjiang
   holdingRegisters[0] = thermocoupleRead()*10 ;


 
  
}

static void sensorsRead(void){

  sensorsReadTemperature();
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

vTaskDelay(100 / portTICK_PERIOD_MS);  
  }
}

void TaskAnalogRead(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
    // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);


  for (;;)
  {
    digitalWrite(LED_BUILTIN,! digitalRead(LED_BUILTIN));   

   sensorsRead();

    vTaskDelay(200 / portTICK_PERIOD_MS);  

  }
}



