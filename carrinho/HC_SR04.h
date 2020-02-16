#ifndef HC_SR04_H
#define HC_SR04_H

#include <Arduino.h>

#define CM true
#define INCH false

enum class HC_SR04_State {
  never_executed, started, interrupted , finished;
}

class HC_SR04 {
  public:
    HC_SR04(int trigger, int echo);
    
    void begin() {
        _state = never_executed;
        _time = -1;
    }
    void start() {
      _time=micros();
      _state = HC_SR04_State::started;
    }
    bool isFinished() { 
      return _finished; 
      }
    unsigned int getRange(bool units=CM);
    static HC_SR04* instance(){ return _instance; }
    
  private:
    static void _echo_isr();
    
    int _trigger, _echo;
    volatile unsigned long _time;
    volatile HC_SR04_State _state;
    static HC_SR04* _instance;
};

#endif
