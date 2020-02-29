#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include <Arduino.h>
#include "analogWrite.h"
#include "rdm6300.h"
#include "AsyncUltrasonic.h"
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

AsyncUltrasonic distanceSensor(PIN_TRIGGER, PIN_ECHO);
Rdm6300 rdm6300;
Motor motorEsq(14, 27, 26);
Motor motorDir(32, 33, 25);
LiquidCrystal_I2C lcd(0x27, 16, 2);


void setup() {
    Serial.begin(115200);
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

    distanceSensor.begin();
}

float velocidade = 0;
float dist = 0;
uint32_t tag = 0;

bool sensor_direita;
bool sensor_esquerda;
bool sensor_central;
bool sensor_lat_dir;
bool sensor_lat_esq;
bool sensor_parada;


void atualizaSensores() {
    sensor_direita = digitalRead(Sensor_direita);
    sensor_esquerda = digitalRead(Sensor_esquerda);
    sensor_central = digitalRead(Sensor_central);
    sensor_lat_dir = digitalRead(Sensor_lat_dir);
    sensor_lat_esq = digitalRead(Sensor_lat_esq);
    sensor_parada = !digitalRead(Sensor_parada);
    
    dist = distanceSensor.getDistance();
  
    rdm6300.update();
    if (rdm6300.is_tag_near()) {
        uint32_t _tag = rdm6300.get_tag_id();
        if (_tag) {
            tag = _tag;
            lcd.setCursor(0, 1);
            lcd.printf("%x", tag);
        }
    } else if (tag) {
        tag = 0;
        lcd.setCursor(0, 1);
        lcd.printf("------");
    }
}

#define fsm_andando 0
#define fsm_parando_na_casinha 1
#define fsm_avancando_na_casinha 2
#define fsm_re_na_casinha 3
#define fsm_esperando_na_casinha 4
#define fsm_saindo_da_casinha 5

#define CARD_NONE 0
#define CARD_OTHER 1
#define CARD_MINE 2



int card = CARD_NONE;
int estadoAtual = fsm_andando;
auto estadoAtualDesde = millis();

int tempoNoEstadoAtual() {
    return millis() - estadoAtualDesde;
}

void setState(int newState) {
    estadoAtual = newState;
    estadoAtualDesde = millis();
}


#define TEMPO_FREIO 1000
#define TEMPO_SAIDA 1000
#define TEMPO_NA_CASINHA 3000
#define TEMPO_NA_CASINHA_ERRADA 1000
#define TEMPO_MAX_RE 3000

//Pinos de velocidade
#define SPEED_FULL     1.0f
#define SPEED_SLOWDOWN 0.3f

#define DIST_SLOWDOWN 20
#define DIST_FULLSTOP 10

#define TAG_CASINHA1 0x786b1f
#define TAG_CASINHA2 0x473d10

void atualizaEstado() {
/*
    - FREIA por 500ms
    - Anda pra trás até alinhar
    - Espera um pouquinho
    - vai embora
*/
    // Já leu a tag desta casinha, e não é a casinha que me interessa
    if (tag == TAG_CASINHA1 || tag == TAG_CASINHA2) {
        if (card != CARD_MINE) {
            card = CARD_MINE;
            Serial.printf("meu cartao: %x\n", tag);
        }
    } else if (tag) {
        if (card != CARD_MINE && card != CARD_OTHER) {
            card = CARD_OTHER;
            Serial.printf("outro cartao: %x\n", tag);
            if (estadoAtual != fsm_andando && estadoAtual != fsm_saindo_da_casinha) {
                Serial.printf("ignorando esta casinha\n");
                setState(fsm_saindo_da_casinha);
            }
        }
    }
    
    switch (estadoAtual) {
        case fsm_andando: {
            if (card != CARD_NONE) {
                card = CARD_NONE;
                Serial.printf("Ignorando cartao\n");
            }
            velocidade = SPEED_FULL;
            if (sensor_parada) {
                Serial.printf("Achei uma casinha! Parou!\n");
                setState(fsm_parando_na_casinha);
            }
            break;
        }
        case fsm_parando_na_casinha: {
            velocidade = 0;
            if (tempoNoEstadoAtual() >= TEMPO_FREIO) {
                Serial.printf("Avançando devagar...\n");
                setState(fsm_avancando_na_casinha);
            }
            break;
        }
        case fsm_avancando_na_casinha: {
            velocidade = SPEED_SLOWDOWN;
            if (!sensor_parada) {
                setState(fsm_re_na_casinha);
                /*if (meuCartao) {
                    Serial.printf("Ops, passou! -- Voltando\n");
                    setState(fsm_re_na_casinha);
                } else {
                    Serial.printf("Casinha errada -- bora!\n");
                    setState(fsm_saindo_da_casinha);
                }*/
                setState(fsm_re_na_casinha);
            }
            break;
        }

        case fsm_re_na_casinha: {
            velocidade = -SPEED_SLOWDOWN;
            if (sensor_parada) {
                // Chegou
                Serial.printf("Voltei para a casinha!\n");
                setState(fsm_esperando_na_casinha);
            } else if (tempoNoEstadoAtual() >= TEMPO_MAX_RE) {
                Serial.printf("Uai, perdi a casinha! -- Acelerando\n");
                setState(fsm_andando);
            }
            break;
        }

        case fsm_esperando_na_casinha: {
            velocidade = 0;
            if (card == CARD_MINE && tempoNoEstadoAtual() >= TEMPO_NA_CASINHA) {
                Serial.printf("Passei tempo suficiente na casinha -- Saindo\n");
                setState(fsm_saindo_da_casinha);
            }
            if (card == CARD_NONE && tempoNoEstadoAtual() >= TEMPO_NA_CASINHA_ERRADA) {
                Serial.printf("Não li meu cartao nesta casinha -- Saindo\n");
                setState(fsm_saindo_da_casinha);
            }
            break;
        }

        
        case fsm_saindo_da_casinha: {
            velocidade = SPEED_FULL;
            if (sensor_parada) {
                setState(fsm_saindo_da_casinha);
            } else if (tempoNoEstadoAtual() >= TEMPO_SAIDA) {
                Serial.printf("Saiu -- Acelerando\n");
                setState(fsm_andando);
            }
            break;
        }
    }
}

void segueLinha() {
     float v = velocidade;
    // Reduz velocidade quando detecta faixas laterais
    if (sensor_lat_dir || sensor_lat_esq) {
        if (v > SPEED_SLOWDOWN) {
            v = SPEED_SLOWDOWN;
        } else if (v < -SPEED_SLOWDOWN) {
            v = -SPEED_SLOWDOWN;
        }
    }

    // Reduz velocidade quando detecta obstáculo com sensor ultrasonico
    if (v > 0) {
        if (dist < DIST_FULLSTOP) {
            v = 0;
        } else if (dist < DIST_SLOWDOWN) {
            v *= ((dist - DIST_FULLSTOP) / (DIST_SLOWDOWN - DIST_FULLSTOP));
        }
    }

    if (sensor_direita == 1 && sensor_esquerda == 0) {
        motorEsq.setSpeed(+1);
        motorDir.setSpeed(-1);
    } else if (sensor_direita == 0 && sensor_esquerda == 1) {
        motorEsq.setSpeed(-1);
        motorDir.setSpeed(+1);
    } else if (v == 0) {
        motorEsq.stop();
        motorDir.stop();
    } else {
        motorEsq.setSpeed(v);
        motorDir.setSpeed(v);
    }  
}

void loop() {
    atualizaSensores();
    atualizaEstado();
    segueLinha();

    lcd.setCursor(0, 0);
    lcd.printf("%d %3d %3d %d      ", sensor_parada, (int)dist, (int)(100*velocidade), estadoAtual);
}
