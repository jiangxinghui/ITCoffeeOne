#include "pressure_sensor.h"
#include "pindef.h"
#include "ADS1X15.h"

#include "i2c_bus_reset.h"

#if defined SINGLE_BOARD
ADS1015 ADS(0x48);
#else
ADS1115 ADS(0x48);
#endif

float previousPressure;
float currentPressure;

void adsInit(void) {

  Wire.begin(); //added by itcoffee


   ADS.begin();
   ADS.setGain(0);      // 6.144 volt
   ADS.setDataRate(4);  // fast


}

float getPressure(void) {  //returns sensor pressure data
  // voltageZero = 0.5V --> 25.6 (8 bit) or 102.4 (10 bit) or 2666.7 (ADS 15 bit)
  // voltageMax = 4.5V --> 230.4 (8 bit) or 921.6 (10 bit) or 24000 (ADS 15 bit)
  //  voltageMax=2.9v                                         ->15500(ads15 bit)
  // range 921.6 - 102.4 = 204.8 or 819.2 or 21333.3
  // pressure gauge range 0-1.2MPa - 0-12 bar
  // 1 bar = 17.1 or 68.27 or 1777.8
#ifdef STM32_BOARD
  getAdsError();
#endif

  if(!ADS.isConnected())
  {
    //Serial.println("not connected");

    return -1;
  }
if(ADS.isConnected()&&ADS.isBusy())
{
  //Serial.println("busy");
  return -2;
}

  previousPressure = currentPressure;
// #if defined STM32_BOARD

// int16_t val=ADS.getValue();

//   currentPressure = (val - 2666) / 1777.8f; // 16bit

// #else



volatile uint16_t val=ADS.readADC(0);


 if(val<2666)val=2666;
 #ifdef VIN_29
   currentPressure = (((val-2666)*2.9/(15500-2666))/2-0.25 )*10 ; //ads input ref=2.9v ,also,pressure transducer input=2.9v, p=v/2-0.25 , v=(val-2666)*2.9/(15500-2666),mpar->bar *10
   #else
   currentPressure = (val- 2666) / 1777.8f; // 16bit
   #endif
// #endif

  return currentPressure;
}

void getAdsError(void) {
  // Reset the hw i2c to try and recover comms
  // on fail to do so throw error
  i2cResetState();



  // Throw error code on ADS malfunction/miswiring
  // Invalid Voltage error code: -100
  // Invalid gain error code: 255
  // Invalid mode error code: 254
 short result = ADS.getError();
  if (result == 0) return;
  char tmp[25];
  unsigned int check = snprintf(tmp, sizeof(tmp), "ADS error code: %i", result);
  if (check > 0 && check <= sizeof(tmp)) {
   // lcdShowPopup(tmp);
  }
}

// //Serial.print(digitalRead(PIN_SCL));    //should be HIGH
// //Serial.println(digitalRead(PIN_SDA));   //should be HIGH, is LOW on stuck I2C bus
// // ERROR CODE 1: I2C bus error. Could not clear sclPin clock line held low
// // ERROR CODE 2: I2C bus error. Could not clear. sclPin clock line held low by slave clock for > 2sec
// // ERROR CODE 3: I2C bus error. Could not clear. sdaPin data line held low
void i2cResetState(void) {

  if (digitalRead(PIN_WIRE_SDA) != HIGH || digitalRead(PIN_WIRE_SCL) != HIGH ||!ADS.isConnected() ) {
  //  LOG_INFO("Reset I2C pins");
    short result = I2C_ClearBus(PIN_WIRE_SDA, PIN_WIRE_SCL);
    char tmp[25];
    unsigned int check = snprintf(tmp, sizeof(tmp), "I2C error code: %i", result);
    if (check > 0 && check <= sizeof(tmp)) {
     if(  result == 0 ) {
      adsInit();
     }else
     {
     ;// {;//lcdShowPopup(tmp);};
    }


    }
    delay(50);
  }
}
