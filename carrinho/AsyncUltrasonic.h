#pragma once

#include <Arduino.h>
#include <inttypes.h>

#define CM true
#define INCH false


class AsyncUltrasonic {
    enum class State {
      trig_off, trig_on, waiting_response, measuring
    };
  
    int pin_trigger, pin_echo;
    volatile State state;
    volatile uint32_t timestamp;
    volatile int32_t measurementUs;

    void isr();
    
public:
    AsyncUltrasonic(int trigger, int echo);
    
    void begin();
    void end();
    float getDistance();
};
