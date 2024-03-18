/* 09:32 15/03/2023 - change triggering comment */
#include "HardwareSerial.h"

#ifndef PINDEF_H
#define PINDEF_H


#define VIN_29

//#define STM32_BOARD
//#define Simulate_Input
#if defined(STM32_BOARD)
// STM32F4 pins definitions
#define thermoDO      PB4  //nano a3


 #define thermoDI      PA7 // not used
#define thermoCS      PA6   //nano d9
#define thermoCLK     PA5   //nano d8

#define thermoDO_2      PB14 // not used
#define thermoCS_2      PB13   //nano d9
#define thermoCLK_2     PB12   //nano d8

 #define zcPin         PA0
 #define brewPin      PA4  // PC14  //nano Rst->D7
// #define relayPin      PA15
#define dimmerPin     PA1

 #define HeaterPin     PA15 //nano a1
// #define steamPin      PC15
// #define valvePin      PB1  //changed by xhjiang from pc13->pb1

// //单板是指 单一pcb板子。和lego的配置有所不同。

// #if defined(SINGLE_BOARD)
// #define waterPin      PB15
// #else
// #define waterPin      PA12
// #endif

// #ifdef PCBV2
// // PCB V2
// #define steamValveRelayPin PB12
// #define steamBoilerRelayPin PB13
// #endif

// #define HX711_sck_1   PB0
// #define HX711_dout_1  PB8
// #define HX711_dout_2  PB9

 //#define USART_LCD     Serial2 // PA2(TX) & PA3(RX)
 
 #define USART_Modbus     Serial2 // PA2(TX) & PA3(RX)
// //#define USART_ESP     Serial1 // PA9(TX) & PA10(RX)

// #define USART_DEBUG   Serial1  // USB-CDC (Takes PA8,PA9,PA10,PA11)

#define BluetoothSerial_TX  PB1 //PB13
#define BluetoothSerial_RX  PA7 //PB14

#define DebugSerial_TX  PB1 // nano d12 , 
#define DebugSerial_RX  PA7 // nano d10 ,

//add by xhjiang
//#define USART_Modbus Serial  //pa9 pa10

#define VR PB0

#else
//arduino uno pwm pin 3,5,6,9,10,11

#define HeaterPin 15  //nano A1



#define thermoDO      10  //nano d10

#define thermoCS      9   //nano d9
#define thermoCLK     8   //nano d8



#define bt_txPin       11
#define bt_rxPin     12

#define brewPin       7
#define zcPin         2

#define dimmerPin     3
#define VR A0

//#define DEBUG_WITH_AVR

#endif  //end 




#endif
