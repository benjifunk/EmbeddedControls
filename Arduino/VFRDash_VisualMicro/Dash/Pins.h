#ifndef  PINS_H
#define PINS_H

// pins for tft display
#define _sclk	52
#define _mosi	51
#define _cs		47
#define _dc		48
#define _rst	44

#define ROTARY_PIN1 A0
#define ROTARY_PIN2 A1

#define OLED_DC    11
#define OLED_CS   12
#define OLED_CLK  10
#define OLED_DATA 9

//interrupt pins		
#define interruptPin 2
#define powerKillPin 3
//#define sensorInterrupt 0

//input pins		
#define N_IND        22
#define LEDPIN       6
#define rpmPin		 18

#define DATAPIN    A4
#define CLOCKPIN   A5

const byte button_pin = A3;
const byte dimPin = A2;

#endif // ! PINS_H
