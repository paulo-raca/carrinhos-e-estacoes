#include <Arduino.h>
#include "analogWrite.h"
#include "rdm6300.h"
#include "Ultrasonic.h"

//Pinos driver pontw H L298
#define MotorA_sentido1 33 // IN1
#define MotorA_sentido2 25 // IN2
#define MotorB_sentido1 26 // IN3
#define MotorB_sentido2 27 // IN4
#define MotorA_PWM 32  // Right Motors
#define MotorB_PWM 14 // Left Motors

//Pinos dos sensores
#define Sensor_direita 35 // LSR
#define Sensor_esquerda 15 // LSL
#define Sensor_central 04 // LSC
#define Sensor_lat_dir 13 // SSR
#define Sensor_lat_esq 18 // SSL
#define Sensor_parada 36 // PARAR

//Pinos do ultrasom
#define PIN_TRIGGER 5
#define PIN_ECHO 39

//RFID
#define RXD2 16
#define TXD2 17

//Pinos de velocidade
#define veloc0 0 // Parada
#define veloc1 1000 // Giro interno
#define veloc2 1023 // Full
#define veloc3 1000 // Giro externo
#define veloc4 552 // Reduzida

HC_SR04 ultrassom(PIN_TRIGGER, PIN_ECHO);
Rdm6300 rdm6300;
int direita, esquerda, central, lat_dir, lat_esq, parada;

void setup() {
    Serial.begin(9600);
    rdm6300.begin(RXD2);
    pinMode(MotorA_sentido1, OUTPUT);
    pinMode(MotorA_sentido2, OUTPUT);
    pinMode(MotorB_sentido1, OUTPUT);
    pinMode(MotorB_sentido2, OUTPUT);
    pinMode(MotorA_PWM, OUTPUT);
    pinMode(MotorB_PWM, OUTPUT);
    pinMode(Sensor_direita, INPUT);
    pinMode(Sensor_esquerda, INPUT);
    pinMode(Sensor_central, INPUT);
    pinMode(Sensor_lat_dir, INPUT);
    pinMode(Sensor_lat_esq, INPUT);
    pinMode(Sensor_parada, INPUT);
    
    //SETAR FREQUENCIA EM 1 KHz
    analogWriteFrequency(100);
    //SETAR RESOLUCAO EM 10 Bits
    analogWriteResolution(10); 
}


#define SPEED_FORWARD 700
#define SPEED_TURN_IN 600
#define SPEED_TURN_OUT 600
void loop() {
    direita = digitalRead(Sensor_direita);
    esquerda = digitalRead(Sensor_esquerda);
    central = digitalRead(Sensor_central);
    lat_dir = digitalRead(Sensor_lat_dir);
    lat_esq = digitalRead(Sensor_lat_esq);
    parada = digitalRead(Sensor_parada);


    Serial.print(lat_esq);
    Serial.print(esquerda);
    Serial.print(central);
    Serial.print(direita);
    Serial.print(lat_dir);
    Serial.print("  ");
    Serial.print(parada);
    Serial.println();

    if (direita == 0 && esquerda == 0) {
      //Vai reto
      digitalWrite(MotorA_sentido1, LOW);
      digitalWrite(MotorA_sentido2, HIGH);
      analogWrite(MotorA_PWM, SPEED_FORWARD, 1023);
      digitalWrite(MotorB_sentido1, HIGH);
      digitalWrite(MotorB_sentido2, LOW);
      analogWrite(MotorB_PWM, SPEED_FORWARD, 1023);
    } else if (direita == 1 && esquerda == 0) {
      //Vira pra Esquerda
      digitalWrite(MotorA_sentido1, HIGH);
      digitalWrite(MotorA_sentido2, LOW);
      analogWrite(MotorA_PWM, SPEED_TURN_IN, 1023);
      digitalWrite(MotorB_sentido1, HIGH);
      digitalWrite(MotorB_sentido2, LOW);
      analogWrite(MotorB_PWM, SPEED_TURN_OUT, 1023);
    } else if (direita == 0 && esquerda == 1) {
      //Vira pra Direita 
      digitalWrite(MotorA_sentido1, LOW);
      digitalWrite(MotorA_sentido2, HIGH);
      analogWrite(MotorA_PWM, SPEED_TURN_OUT, 1023);
      digitalWrite(MotorB_sentido1, LOW);
      digitalWrite(MotorB_sentido2, HIGH);
      analogWrite(MotorB_PWM, SPEED_TURN_IN, 1023);
    }
} 
