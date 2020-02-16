#ifndef HC_SR04_H
#define HC_SR04_H

#include <Arduino.h>
#include <inttypes.h>

#define CM true
#define INCH false


class HC_SR04 {
    enum class State {
      trig_off, trig_on, waiting_response, measuring
    };
  
    int pin_trigger, pin_echo;
    volatile State state;
    volatile uint32_t timestamp;
    volatile int32_t measurementUs;
    bool hasNew;


    void isr();
    
public:
    HC_SR04(int trigger, int echo);
    
    void begin();
    void end();
    bool hasNewData();
    float getDistance();

};

#endif
