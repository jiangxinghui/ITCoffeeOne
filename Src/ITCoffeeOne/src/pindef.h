/* 09:32 15/03/2023 - change triggering comment */
#ifndef PINDEF_H
#define PINDEF_H

//#define STM32_BOARD
#define Simulate_Input
#if defined(STM32_BOARD)
// STM32F4 pins definitions
#define thermoDO      PB4
#define thermoDI      PA7 // not used
#define thermoCS      PA6
#define thermoCLK     PA5

// #define zcPin         PA0
// #define brewPin       PC14
// #define relayPin      PA15
 #define HeaterPin     PA1
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

// #define USART_LCD     Serial2 // PA2(TX) & PA3(RX)
// //#define USART_ESP     Serial1 // PA9(TX) & PA10(RX)

// #define USART_DEBUG   Serial1  // USB-CDC (Takes PA8,PA9,PA10,PA11)



//add by xhjiang
#define USART_Modbus Serial1



#else
//arduino uno pwm pin 3,5,6,9,10,11

#define HeaterPin 9

#define thermoDO      4
   
#define thermoCS      5
#define thermoCLK     6




#define bt_rxPin     10
#define bt_txPin       11




#endif  //end stm32 board




#endif
