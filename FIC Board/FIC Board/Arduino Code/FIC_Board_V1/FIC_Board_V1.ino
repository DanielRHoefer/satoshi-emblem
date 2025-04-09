#include <Arduino.h>
#include "SiC45x.h"

SiC45x sic45x(0x5C);

#define LET_POWER1 4
#define LET_POWER2 5
#define LET_POWER3 6
#define LET_POWER4 7
#define LET_POWER5 8
#define LET_POWER6 9
#define RAP_POWER1 10
#define RAP_POWER2 11
#define RAP_POWER3 12
#define RAP_POWER4 13

#define LET_COLOR1 22
#define LET_COLOR2 23
#define LET_COLOR3 24
#define LET_COLOR4 25
#define LET_COLOR5 26
#define LET_COLOR6 27
#define RAP_COLOR1 28
#define RAP_COLOR2 29
#define RAP_COLOR3 30
#define RAP_COLOR4 31

#define TURN_P A0
#define TURN_D A1
#define MISC_1 A2
#define MISC_2 A3

int LET_POWER_PIN[6] = {4,5,6,7,8,9};
int RAP_POWER_PIN[4] = {10,11,12,13};
int LET_COLOR_PIN[6] = {22,23,24,25,26,27};
int RAP_COLOR_PIN[4] = {28,29,30,31};

float vout = 6.5; //MAXIMUM OUTPUT VOLTAGE (DO NOT SET ABOVE 7.4V)

boolean c = false;

int state = 0;
boolean action = 0;
byte offAction = 0;

long state_timer = 0;

int animationDelay = 30;

void setup() {
  Serial.begin(115200);   //begin serial communication for debugging

  //wait for a serial connection for debugging
  while(!Serial) {
    delay(10);
  }

  //set all I/O configurations
  for(int i=0; i<6; i++){
    pinMode(LET_POWER_PIN[i], OUTPUT);
    pinMode(LET_COLOR_PIN[i], OUTPUT);
  }
  for(int i=0; i<4; i++){
    pinMode(RAP_POWER_PIN[i], OUTPUT);
    pinMode(RAP_COLOR_PIN[i], OUTPUT);
  }

  pinMode(TURN_D, INPUT); //inputs    
  pinMode(TURN_P, INPUT);
  pinMode(MISC_1, INPUT);
  pinMode(MISC_2, INPUT);

  sic45x.begin();

  sic45x.sendClearFaults();
  sic45x.printStatusWord();

  sic45x.setOperation(
    SIC45X_OPERATION_ON_OFF_DISABLED
    | SIC45X_OPERATION_OFF_B_IMMEDIATE
    | SIC45X_OPERATION_MARGIN_COMMAND
    | SIC45X_OPERATION_MRGNFLT_FOLLOW
  );

  //sic45x.setFrequencySwitch(1000);
  sic45x.setInterleave(SIC45X_INTERLEAVE_MODE_MASTER);

  sic45x.setVoutOvFaultLimit(vout + 1.2);
  sic45x.setVoutOvWarnLimit(vout + 1.1);
  sic45x.setVoutUvWarnLimit(vout - 1.1);
  sic45x.setVoutUvFaultLimit(vout - 1.2);
  sic45x.setVoutUvFaultResponse(
    SIC45X_VOUT_UV_FAULT_RESPONSE_UVRSP_CONTINUE // this basically ignores faults
    | SIC45X_VOUT_UV_FAULT_RESPONSE_UVRTY_NO_RESTART
    | SIC45X_VOUT_UV_FAULT_RESPONSE_UVDLY_NO_DELAY
  );
  sic45x.setVoutScaleLoop(SIC45X_VOUT_SCALE_LOOP_5V0_12V0);
  sic45x.setVoutCommand(vout);
  sic45x.setPowerGoodOn(vout - 0.3);
  sic45x.setPowerGoodOff(vout - 0.6);
  
  sic45x.setVinOvFaultLimit(20);

  sic45x.setOnOffConfiguration(
    SIC45X_ON_OFF_CONFIGURATION_PU_COMMAND
    | SIC45X_ON_OFF_CONFIGURATION_CMD_RESPOND
    | SIC45X_ON_OFF_CONFIGURATION_EN_IGNORE
    | SIC45X_ON_OFF_CONFIGURATION_ENPOL_HIGH
    | SIC45X_ON_OFF_CONFIGURATION_OFFB1_IMMEDIATE
  );

  sic45x.printStatusWord();

  //initial startup states
  for(int i=0; i<6; i++){
    digitalWrite(LET_POWER_PIN[i], LOW);
    digitalWrite(LET_COLOR_PIN[i], LOW);
  }
  for(int i=0; i<4; i++){
    digitalWrite(RAP_POWER_PIN[i], LOW);
    digitalWrite(RAP_COLOR_PIN[i], LOW);
  }

}

void loop() {
    sic45x.setOperation(
      SIC45X_OPERATION_ON_OFF_ENABLED
      | SIC45X_OPERATION_OFF_B_IMMEDIATE
      | SIC45X_OPERATION_MARGIN_COMMAND
      | SIC45X_OPERATION_MRGNFLT_FOLLOW
    );

    if(state != 4 && state != 5) {
      if(digitalRead(TURN_D) == LOW && digitalRead(TURN_P) == LOW) {
        state = 0;    //normal condition
        if(offAction == 1)
          state = 4;
        if(offAction == 2)
          state = 5;
        action = 0;
          
      }
      if(digitalRead(TURN_D) == HIGH && digitalRead(TURN_P) == LOW) {
        state = 1;    //left hand turn
        if(action == 0) {
          state_timer = millis();
          action = 1;
        }
        offAction = 1;
      }
      if(digitalRead(TURN_D) == LOW && digitalRead(TURN_P) == HIGH) {
        state = 2;    //right hand turn
        if(action == 0) {
          state_timer = millis();
          action = 1;
        }
        offAction = 2;
      }
      if(digitalRead(TURN_D) == HIGH && digitalRead(TURN_P) == HIGH) {
        state = 3;    //hazards
        if(action == 0) {
          state_timer = millis();
          action = 1;
        }
      }
    }

    byte allONL[6] = {255,255,255,255,255,255};
    byte allONR[4] = {255,255,255,255};
    byte rapLeft[4] = {255,255,0,0};
    byte rapRight[4] = {0,0,255,255};
    byte allOFFR[4] = {0,0,0,0};
    
    if(state == 0) {
      updateLetters(allONL, B000000);
      updateRaptors(allONR, B0000);

    }
    if(state == 1) {
      updateRaptors(allONR, B1111);
      long stateDuration = millis() - state_timer;

      if(stateDuration < animationDelay*1)
        updateLetters(allONL, B100000);
      else if(stateDuration < animationDelay*2)
        updateLetters(allONL, B110000);
      else if(stateDuration < animationDelay*3)
        updateLetters(allONL, B111000);
      else if(stateDuration < animationDelay*4)
        updateLetters(allONL, B111100);
      else if(stateDuration < animationDelay*5)
        updateLetters(allONL, B111110);
      else if(stateDuration < animationDelay*6)
        updateLetters(allONL, B111111);
    }
    if(state == 2) {

      updateRaptors(allONR, B1111);
      long stateDuration = millis() - state_timer;

      if(stateDuration < animationDelay*1)
        updateLetters(allONL, B000001);
      else if(stateDuration < animationDelay*2)
        updateLetters(allONL, B000011);
      else if(stateDuration < animationDelay*3)
        updateLetters(allONL, B000111);
      else if(stateDuration < animationDelay*4)
        updateLetters(allONL, B001111);
      else if(stateDuration < animationDelay*5)
        updateLetters(allONL, B011111);
      else if(stateDuration < animationDelay*6)
        updateLetters(allONL, B111111);
    }
    if(state == 3) {
      updateLetters(allONL, B111111);
      updateRaptors(allONR, B1111);
    }
    if(state == 4) {
      long stateDuration = millis() - state_timer;
      if(stateDuration > 1500)  {
        offAction = 0;
        state = 0;
      }

      if(digitalRead(TURN_D) == HIGH || digitalRead(TURN_P) == HIGH)  {
        state = 0;
      }
      updateLetters(allONL, B000000);
      updateRaptors(rapRight, B1111);
    }
    if(state == 5) {
      long stateDuration = millis() - state_timer;
      if(stateDuration > 1500)  {
        offAction = 0;
        state = 0;
      }

      if(digitalRead(TURN_D) == HIGH || digitalRead(TURN_P) == HIGH)  {
        state = 0;
      }
      updateLetters(allONL, B000000);
      updateRaptors(rapLeft, B1111);
    }

    Serial.print("state: ");
    Serial.println(state);
    Serial.print("analogVoltage: ");
    Serial.println(analogRead(TURN_D));
    
}

void updateLetters(byte power[6], byte color) {
  for (int i = 0; i < 6; i++) {
    analogWrite(LET_POWER_PIN[i], power[i]);
    digitalWrite(LET_COLOR_PIN[i], !bitRead(color, i));
  }
}

void updateRaptors(byte power[4], byte color) {
  for (int i = 0; i < 4; i++) {
    analogWrite(RAP_POWER_PIN[i], power[i]);
    digitalWrite(RAP_COLOR_PIN[i], !bitRead(color, i));
  }
}

void info() {
   // Report operational parameters every 2s
    Serial.print("IN: V: ");
    Serial.print(sic45x.getReadVin());
    Serial.print("V, I: ");
    Serial.print(sic45x.getReadIin());
    Serial.print("A, P: ");
    Serial.print(sic45x.getReadPin());
    Serial.print("W | OUT: V: ");
    Serial.print(sic45x.getReadVout());
    Serial.print("V, I: ");
    Serial.print(sic45x.getReadIout());
    Serial.print("A, P: ");
    Serial.print(sic45x.getReadPout());
    Serial.print("W | temp: ");
    Serial.print(sic45x.getReadTemperature());
    Serial.print("°C, duty cycle: ");
    Serial.println(sic45x.getReadDutyCycle());
}

void pulse() {
  for(int i=0; i<=255; i++) {
      for(int j=0; j<6; j++) {
        analogWrite(LET_POWER_PIN[j], i);
      }
      for(int j=0; j<4; j++) {
        analogWrite(RAP_POWER_PIN[j], i);
      }
      delay(1); 
    }
    delay(200);
    for(int i=255; i>=0; i--) {
      for(int j=0; j<6; j++) {
        analogWrite(LET_POWER_PIN[j], i);
      }
      for(int j=0; j<4; j++) {
        analogWrite(RAP_POWER_PIN[j], i);
      }
      delayMicroseconds(500); 
    }
    //analogWrite(POWER, 0);
    delay(200);

    
    for(int j=0; j<6; j++) {
      digitalWrite(LET_COLOR_PIN[j], c);
    }
      for(int j=0; j<4; j++) {
      digitalWrite(RAP_COLOR_PIN[j], c);
    }
    c = !c;
}

void on() {
  for(int j=0; j<6; j++) {
      analogWrite(LET_POWER_PIN[j], 255);
      digitalWrite(LET_COLOR_PIN[j], 1);
    }
    for(int j=0; j<4; j++) {
      analogWrite(RAP_POWER_PIN[j], 255);
      digitalWrite(RAP_COLOR_PIN[j], 1);
    }
}
