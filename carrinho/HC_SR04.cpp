#include "HC_SR04.h"

//HC_SR04 *HC_SR04::_instance=NULL;
HC_SR04 *HC_SR04::_instance(NULL);

HC_SR04::HC_SR04(int trigger, int echo)
    : _trigger(trigger), _echo(echo), _finished(false)
{
  if(_instance==0) _instance=this;    
}

void HC_SR04::begin(){
  pinMode(_trigger, OUTPUT);
  digitalWrite(_trigger, LOW);
  pinMode(_echo, INPUT);  
  attachInterrupt(_echo, _echo_isr, CHANGE);
}

void HC_SR04::start(){
  _finished=false;
  _start=micros();
  digitalWrite(_trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(_trigger, LOW);  
}

bool HC_SR04::finished() {
  if (!_finished) {
    unsigned long now = micros();
    unsigned long elapsed = now - _start;
    if (elapsed > 100000) {
      _end = _start + 100000;
      _finished = true;
    }
  }
  return _finished;
}

unsigned int HC_SR04::getRange(bool units){
  return (_end-_start)/((units)?58:148);
}

void HC_SR04::_echo_isr(){
  HC_SR04* _this=HC_SR04::instance();
  
  switch(digitalRead(_this->_echo)){
    case HIGH:
       if (!_this->_finished) {
           _this->_start=micros();
      }
      break;
    case LOW:
      if (!_this->_finished) {
      _this->_end=micros();
      _this->_finished=true;
      }
      break;
  }   
}
