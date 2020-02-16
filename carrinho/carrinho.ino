#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include <Arduino.h>
#include "analogWrite.h"
#include "rdm6300.h"
#include "HC_SR04.h"
#include "Motor.h"

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
Motor motorEsq(14, 27, 26);
Motor motorDir(32, 33, 25);
LiquidCrystal_I2C lcd(0x27, 16, 2);


void setup() {
    Serial.begin(9600);
    rdm6300.begin(RXD2);
    Wire.begin();
    lcd.backlight();
    lcd.begin(16,2);
    
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

    ultrassom.begin();
    ultrassom.start();
}


enum Direcao {
  RETO,
  ESQ,
  DIR
};

float velocidade = .3;
float dist = 100;
Direcao direcao = RETO;

void loop() {
  if (ultrassom.isFinished()) {
    dist = ultrassom.getRange();
    Serial.printf("Dist=%f\n", dist);
    ultrassom.start();
  }

  bool direita = digitalRead(Sensor_direita);
  bool esquerda = digitalRead(Sensor_esquerda);
  bool central = digitalRead(Sensor_central);
  bool lat_dir = digitalRead(Sensor_lat_dir);
  bool lat_esq = digitalRead(Sensor_lat_esq);
  bool parada = digitalRead(Sensor_parada);

  if (lat_dir == 1 && lat_esq == 1) {
    //Desacelera
    velocidade = .3;
  } else if (lat_dir == 1 || lat_esq == 1) {
    velocidade = .35;
  } else {
    velocidade = 1;
  }

  //  35cm -- Vel maxima
  //  10cm -- Motor desligado
  // <10cm -- Motor freiando
  if (dist < 10) {
    motorEsq.slowDown();
    motorDir.slowDown();
    velocidade = 0;
  } else if (dist < 35) {
    velocidade *= ((dist - 10) / 25.);
  }
  lcd.setCursor(0, 0);
  lcd.print("vel=");
  lcd.print(velocidade);
  lcd.setCursor(0, 1);
  lcd.print("dist=");
  lcd.print(dist);
  

  if (direita == 0 && esquerda == 0) {
    direcao = RETO;
  } else if (direita == 1 && esquerda == 0) {
    direcao = DIR;
  } else if (direita == 0 && esquerda == 1) {
    direcao = ESQ;
  }

  direcao = RETO;
  switch (direcao) {
    case RETO:
      motorEsq.setSpeed(velocidade);
      motorDir.setSpeed(velocidade);
      break;
    case DIR:
      motorEsq.setSpeed(+1);
      motorDir.setSpeed(-1);
      break;
    case ESQ:
      motorEsq.setSpeed(-1);
      motorDir.setSpeed(+1);
      break;
  }
} 
