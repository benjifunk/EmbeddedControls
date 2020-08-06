

/*
- Take signal from front wheel sensor and multiply by circumference of front tire to get speed.
- Map speed to stepper position/servo rotation angle
- Record distance traveled:
  + Odometer
  + Trip(s)
  (write to memory upon loss of power)
 - Display Odo... etc
  + Button to display when key on but no tach (~2s)
  + Tach sense to turn on Odo when bike is running
  + Display kill sequence on sense power == low

= External interrupt is used to detect wheel RPM (Use hall effect sensor on rotor - 8 'blades' per rev.)
= Use time differential to calculate revolutions per millisecond, convert to rpm (*60 s/m) :
   => rpm = (60/2)*(1000/(millis() - prevTime))*rev/blades
   => Speed = rpm * circumference * 12 / 60
*/
#include "Arduino.h"
#include "avr/interrupt.h"

int sensorPin = 2;
unsigned long previousMillis = 0;

const float circum = 72.45; // circumference in inches
      float mph = 0,
            dist = 0;

volatile int _sensor_ticks = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(sensorPin), sensor_ISR, RISING);

  Serial.println("Setup Done.\n");
}

void sensor_ISR()
{
  _sensor_ticks++;
}

void loop()
{  
 unsigned long currentMillis = millis(); // get the current time
 if(_sensor_ticks > 0) // if the wheel is moving
  {
    if(_sensor_ticks < 5 || curentMillis - previousMillis > 100) // smoothing
    {
      calc_Speed(currentMillis);
    }
  }
  else 
  {
    mph = 0;
  }
  _sensor_ticks = 0;
}

void calc_Speed(unsigned long cM)
{
  unsigned long diff = cM-previousMillis;
  noInterrupts();
  dist = _sensor_ticks*circum/63360;
  mph = (float)3600000*dist/(diff);
  
  Serial.print(_sensor_ticks);
  Serial.print("  ");
  
  Serial.print(mph);
  Serial.print(" MPH     \r");
  previousMillis = cM;
  interrupts();
}

