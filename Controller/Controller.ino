#include <Wire.h>
#include <IRremote.h>
#include "RTClib.h"

RTC_DS1307 RTC;

IRsend irsend;

int IR_CODE_LEN = 32;
unsigned long IR_CODE_PWR = 0x48B7C03F;
unsigned long IR_CODE_UP = 0x48B7906F;
unsigned long IR_CODE_DOWN = 0x48B7708F;
unsigned long IR_CODE_PLASMA = 0x48B700FF;
unsigned long IR_CODE_TURBO = 0x48B708F7;

typedef enum {FAN_OFF, FAN_AUTO, FAN_LOW, FAN_MEDIUM, FAN_HIGH, FAN_TURBO} fan_state;
fan_state curFanState = FAN_OFF;

typedef enum {PLASMA_OFF, PLASMA_ON} plasma_state;
plasma_state curPlasmaState = PLASMA_OFF;

void setup() {
    Serial.begin(57600);
    Wire.begin();
    RTC.begin();
    
    setAuto();
}

void loop () {
  DateTime now = RTC.now();
 
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
 
  delay(5000);
  

  if(now.hour() > 21 || now.hour() < 8)
  {
    if(curFanState != FAN_LOW) //All these checks are redundant but they make me feel warm inside for now
      setLow();
    
    if(curPlasmaState != PLASMA_OFF)
      disablePlasma();
  }
  else if((now.day() > 1 && now.day() < 7 && now.hour() == 8) || (now.day() == 1 || now.day() == 7 && now.hour() == 10))
  {
    Serial.println("TURBO TIME");
    
    if(curFanState != FAN_TURBO)
      setTurbo();
    
    if(curPlasmaState != PLASMA_ON)
      enablePlasma();    
  }
  else if(now.hour() == 15)
  {
    Serial.println("HIGH TIME");
    
    if(curFanState != FAN_HIGH)
      setHigh();
    
    if(curPlasmaState != PLASMA_ON)
      enablePlasma();      
  }
  else
  {
    if(curFanState != FAN_AUTO)
      setAuto();
    
    if(curPlasmaState != PLASMA_OFF)
      enablePlasma();   
  }
}

void sendCode(unsigned long codeValue){
  Serial.println("SEND CODE");
  irsend.sendNEC(codeValue, IR_CODE_LEN);
  delay(2000);
}

void disablePlasma(){
  if(curPlasmaState == PLASMA_ON){
    sendCode(IR_CODE_PLASMA);
    curPlasmaState = PLASMA_OFF;
    Serial.println("PLASMA DISABLED");
  }    
}

void enablePlasma(){
  if(curPlasmaState == PLASMA_OFF){
    sendCode(IR_CODE_PLASMA);
    curPlasmaState = PLASMA_ON;
    Serial.println("PLASMA ENABLED");
  }  
}

void setAuto(){
  //From the remote the only way to go back into AUTO is turn off then back on
  if(curFanState != FAN_AUTO){
    turnOff();
    turnOn();
    Serial.println("AUTO SET");
  }
}

void setLow(){
  if(curFanState != FAN_LOW){
    //Worst case is we are on TURBO which takes 3 down presses to get to low
    turnOn();
    sendCode(IR_CODE_DOWN);
    sendCode(IR_CODE_DOWN);
    sendCode(IR_CODE_DOWN);
    sendCode(IR_CODE_DOWN); //Send an extra to be safe since no harm
    curFanState = FAN_LOW;
    Serial.println("LOW SET");
  }
}

void setTurbo(){
  if(curFanState != FAN_TURBO){
    turnOn();
    sendCode(IR_CODE_TURBO);
    curFanState = FAN_TURBO;
    Serial.println("TURBO SET");
  }
}

void setHigh(){
  if(curFanState != FAN_HIGH){
    turnOn();
    sendCode(IR_CODE_TURBO);
    sendCode(IR_CODE_DOWN);
    curFanState = FAN_HIGH;
    Serial.println("HIGH SET");
  }
}

void setMedium(){
  if(curFanState != FAN_MEDIUM){
    turnOn();
    sendCode(IR_CODE_TURBO);
    sendCode(IR_CODE_DOWN);
    sendCode(IR_CODE_DOWN);
    curFanState = FAN_MEDIUM;
    Serial.println("MEDIUM SET");
  }
}

void turnOn(){
  if(curFanState == FAN_OFF){
    sendCode(IR_CODE_PWR);
    curFanState = FAN_AUTO;
    curPlasmaState = PLASMA_ON;
    Serial.println("TURNED ON");
  }
}

void turnOff(){
  if(curFanState != FAN_OFF) {
    sendCode(IR_CODE_PWR);
    curFanState = FAN_OFF;
    curPlasmaState = PLASMA_OFF;
    Serial.println("TURNED OFF");
  }
}

