#include <Arduino.h>
#include "analogWrite.h"

class Motor {
  int pin_en;
  int pin_direction_1;
  int pin_direction_2;
  
public:
  inline Motor(int pin_en, int pin_dir1, int pin_dir2) {
    this->pin_en = pin_en;
    this->pin_direction_1 = pin_dir1;
    this->pin_direction_2 = pin_dir2;
    pinMode(this->pin_en, OUTPUT);
    pinMode(this->pin_direction_1, OUTPUT);
    pinMode(this->pin_direction_2, OUTPUT);
    digitalWrite(this->pin_en, LOW);
  }

  inline void setSpeed(float speed) {  
      if (speed > 1)  speed =  1;
      if (speed < -1) speed = -1;
      if (speed > 0) {
        digitalWrite(this->pin_direction_1, LOW);
        digitalWrite(this->pin_direction_2, HIGH);
        analogWrite(this->pin_en, (int)(speed*1023), 1023);
      } else {
        digitalWrite(this->pin_direction_1, HIGH);
        digitalWrite(this->pin_direction_2, LOW);        
        analogWrite(this->pin_en, (int)(-speed*1023), 1023);
      }
  }
  inline void slowDown(float strengh=1) {
    if (strengh > 1)  strengh =  1;
    if (strengh < 0) strengh = 0;
    digitalWrite(this->pin_direction_1, LOW);
    digitalWrite(this->pin_direction_2, LOW);        
    analogWrite(this->pin_en, (int)(strengh*1023), 1023);
  }
};