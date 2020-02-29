#include "AsyncUltrasonic.h"
#include <FunctionalInterrupt.h>

#define SOUND_SPEED 343.0 //m/s
#define MAX_DIST 10.0 //m
#define ECHO_TIMEOUT (2*MAX_DIST / SOUND_SPEED)


AsyncUltrasonic::AsyncUltrasonic(int trigger, int echo)
    : pin_trigger(trigger), pin_echo(echo), state(State::trig_off), measurementUs(-1)
{ }


void AsyncUltrasonic::begin() {
  pinMode(pin_trigger, OUTPUT);
  pinMode(pin_echo, INPUT_PULLDOWN);
  digitalWrite(pin_trigger, LOW);
  
  uint32_t now = micros();
  state = State::trig_off;
  attachInterrupt(pin_echo, std::bind(&AsyncUltrasonic::isr, this), CHANGE);
}

void AsyncUltrasonic::end() {
  detachInterrupt(pin_echo);
  measurementUs = -1;
  pinMode(pin_trigger, INPUT);
  pinMode(pin_echo, INPUT);
}

static volatile int c1=0, c2=0, c3=0, is0=0, is1=1;

float AsyncUltrasonic::getDistance() {
    uint32_t now = micros();
    uint32_t elapsedMicros = now - timestamp;

    noInterrupts();
   float ret = NAN;
    if (measurementUs >= 0) {
        ret = measurementUs * 100 * SOUND_SPEED / (2 * 1000000);
    }
    
    switch (state) {
        case (State::trig_off):
        case (State::waiting_response):{
            if (elapsedMicros > ECHO_TIMEOUT * 1000000) {
                // Timeout waiting for an echo signal -- trigger again
                digitalWrite(pin_trigger, HIGH);
                state = State::trig_on;
                timestamp = now;
            }
            break;
        }
        case (State::trig_on): {
            if (elapsedMicros > 10) {
                digitalWrite(pin_trigger, LOW);  
                state = State::waiting_response;
                timestamp = now;
            }
            break;
        }

    }
        
 
    interrupts();
    return ret;
}

void IRAM_ATTR AsyncUltrasonic::isr() {
    bool pinState = digitalRead(pin_echo);
    c1++;
    if (state == State::waiting_response && pinState) {
        c2++;
        timestamp = micros();
        state = State::measuring;
        
    } else if (state == State::measuring && !pinState) {
        c3++;
        uint32_t now = micros();
        measurementUs = now - timestamp;
        timestamp = now;
        digitalWrite(pin_trigger, HIGH);
        state = State::trig_on;
    }
}
