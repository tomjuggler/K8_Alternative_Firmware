

/*
  Copyright (c) 2014-2015 NicoHood
  See the readme for credit to other people.

  IRL Receive

  Receives IR signals from different protocols and prints them to the Serial monitor.
  Choose your protocols that should be decoded. Remove the not used ones to save flash/ram/speed.
  You can choose a custom debounce time to not trigger a button two times in a row too fast.

  The following pins are usable for PinInterrupt or PinChangeInterrupt*:
  Arduino Uno/Nano/Mini: All pins are usable
  Arduino Mega: 10, 11, 12, 13, 50, 51, 52, 53, A8 (62), A9 (63), A10 (64),
              A11 (65), A12 (66), A13 (67), A14 (68), A15 (69)
  Arduino Leonardo/Micro: 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI)
  HoodLoader2: All (broken out 1-7) pins are usable
  Attiny 24/44/84: All pins are usable
  Attiny 25/45/85: All pins are usable
  Attiny 13: All pins are usable
  Attiny 441/841: All pins are usable
  ATmega644P/ATmega1284P: All pins are usable

  PinChangeInterrupts* requires a special library which can be downloaded here:
  https://github.com/NicoHood/PinChangeInterrupt
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//In order to get this to work had to make changes to: /home/tom/.arduino15/packages/attiny/hardware/avr/1.0.2/variants/tiny8/pins_arduino.h
//notes in the source, added the following line:
//added below myself (TOM) - helps compile IRLRemote code (but how does it work??????)
//#define digitalPinToInterrupt(p) ((p) == 2 ? 0 : ((p) == 3 ? 1 : NOT_AN_INTERRUPT))
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// include PinChangeInterrupt library* BEFORE IRLremote to acces more pins if needed
//#include "PinChangeInterrupt.h"
//
#include <EEPROM.h>
#include "IRLremote.h"

// Choose a valid PinInterrupt or PinChangeInterrupt* pin of your Arduino board
#define pinIR 2

// Choose the IR protocol of your remote. See the other example for this.
CNec IRLremote;
//CPanasonic IRLremote;
//CHashIR IRLremote;
//#define IRLremote Sony12

//#define pinLed 3

boolean ready = false;
//these are the exact same pins as K8:
//1, 3, 4
//only difference is (maybe) the colours!!!!!
int blueLed = 1; //on right
int greenLed = 3; //middle
int redLed = 4; //left
int delayTime = 25;

int selection = 0;
int maxSelection = 3;

////////////////////////HEX codes:///////////////////////////
static uint8_t greenHEX = 0x9;
static uint8_t redHEX = 0x8;
static uint8_t blueHEX = 0xA;
static uint8_t yellowHEX = 0xB;
static uint8_t cyanHEX = 0xD;
static uint8_t magentaHEX = 0xC;
static uint8_t whiteHEX = 0xE;
static uint8_t fadeHEX = 0xF;
static uint8_t strobeplusHEX = 0x15;
static uint8_t RGBStrobeHEX = 0x14;
static uint8_t rainbowHEX = 0x16;
static uint8_t halfstrobeHEX = 0x17;
static uint8_t BGStrobeHEX = 0x19;
static uint8_t GRStrobeHEX = 0x18;
static uint8_t onHEX = 0x6;
static uint8_t offHEX = 0x7;
static uint8_t nextHEX = 0x4;
static uint8_t previousHEX = 0x5;
static uint8_t demoHEX = 0x1A;
static uint8_t extraHEX1 = 0x1B;
static uint8_t extraHEX2 = 0x11;
static uint8_t extraHEX3 = 0x10;
static uint8_t extraHEX4 = 0x12;
static uint8_t extraHEX5 = 0x13;

unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
long interval = 125;           // interval at which to blink (milliseconds)
int ledState = LOW;
boolean flashy = false;

//volatile uint8_t currentSignal = 0x8; //for flashy
//volatile uint8_t sendSignal = 0x9; //for normal
volatile uint8_t inSignal = 0xA; //for both
volatile uint8_t prevSignal = 0xA; //for memory

uint8_t threeWay = 0;
uint8_t fadeAWay = 0;
uint8_t rainbowWay = 0;
uint8_t plusWay = 0;

void setup()
{
  pinMode(blueLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  //  Serial.begin(115200);
  //  Serial.println(F("Startup"));

  Blue();
  // Start reading the remote. PinInterrupt or PinChangeInterrupt* will automatically be selected
  if (!IRLremote.begin(pinIR)) {
  }
}

void loop()
{

  // Check if we are currently receiving data
  //if (!IRLremote.receiving()) {
  // Run code that disables interrupts, such as some led strips
  //}
//get previous signal for memory:

  // Check if new IR protocol data is available
 
  if (IRLremote.available()) {
    auto data = IRLremote.read();
    if (data.command == 0x0) {

    } else {
     inSignal = data.command;
    }
  }

  if (flashy) {
    flash();
  } else {
    testCommand();
  }
}

void testCommand() {
  if (inSignal == greenHEX) { //Green
    Green();
    prevSignal = inSignal;
  } else if (inSignal == redHEX) { //Red
    Red();
    prevSignal = inSignal;
  } else if (inSignal == blueHEX) { //Blue
    Blue();
    prevSignal = inSignal;
  }  else if (inSignal == yellowHEX) {
    Yellow();
    prevSignal = inSignal;
  } else if (inSignal == cyanHEX) {
    Cyan();
    prevSignal = inSignal;
  } else if (inSignal == magentaHEX) {
    Magenta();
    prevSignal = inSignal;
  } else if (inSignal == whiteHEX) {
    White();
    prevSignal = inSignal;
  } else if (inSignal == fadeHEX) {
    flashy = false; //not working in flashy mode
    Fade();
    prevSignal = inSignal;
  } else if (inSignal == strobeplusHEX) {
    flashy = false; //not working in flashy mode
    Strobeplus();
    prevSignal = inSignal;
  } else if (inSignal == RGBStrobeHEX) {
    flashy = false; //not working in flashy mode
    RGBStrobe();
    prevSignal = inSignal;
  } else if (inSignal == rainbowHEX) {
    flashy = false; //not working in flashy mode
    Rainbow();
    prevSignal = inSignal;
  } else if (inSignal == halfstrobeHEX) {
    flashy = false; //not working in flashy mode
    Halfstrobe();
    prevSignal = inSignal;
  } else if (inSignal == BGStrobeHEX) {
    flashy = false; //not working in flashy mode
    BGStrobe();
    prevSignal = inSignal;
  }else if (inSignal == GRStrobeHEX) {
    flashy = false; //not working in flashy mode
    GRStrobe();
    prevSignal = inSignal;
  } else if (inSignal == nextHEX) {
    Next();
    inSignal = prevSignal; //now switch to whatever we were doing before .. need to test for non-colour signals here though...
  }else if (inSignal == previousHEX) {
    Previous();
    inSignal = prevSignal;
  } else if (inSignal == extraHEX1) {
    Extra1();
    inSignal = prevSignal;
  } else if (inSignal == extraHEX5) {
    Extra5();
    inSignal = prevSignal;
  } else if (inSignal == offHEX) {
    Off();    
  } else if (inSignal == onHEX) {
    inSignal = prevSignal;   
  }


}

void flash() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      Off();
      ledState = HIGH;
    } else {
      testCommand();
      ledState = LOW;
    }
  }

}
//0a
void Red() {
  //  flashy = false; //test
//  Off(); //this is having some sort of adversarial effect, too many concurrent digital writes to the same pin???
  digitalWrite(redLed, HIGH);
  digitalWrite(blueLed, LOW);
  digitalWrite(greenLed, LOW);
}
//1b green
void Green() {
//  Off();
  digitalWrite(greenLed, HIGH);
  digitalWrite(blueLed, LOW);
  digitalWrite(redLed, LOW);
}
//2c blue
void Blue() {
//  Off();
  digitalWrite(blueLed, HIGH);
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);
}
//3d Yellow
void Yellow() {
//  Off();
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, HIGH);
  digitalWrite(blueLed, LOW);
}
//4e Cyan
void Cyan() {
//  Off();
  digitalWrite(blueLed, HIGH);
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);
}
//5f Magenta
void Magenta() {
//  Off();
  digitalWrite(blueLed, HIGH);
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, LOW);
}
//6g White
void White() {
//  Off();
  digitalWrite(blueLed, HIGH);
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, HIGH);
}
//7h Fade
void Fade() {
  //after some testing, on attiny85 Red and Blue pins support pwm but green does not. Need another solution for fading: 
  
  for(int i = 1; i < 1000; i++){
    digitalWrite(blueLed, HIGH);
    delayMicroseconds(i);
    digitalWrite(blueLed, LOW);
    delayMicroseconds(1000 - i);
    
//    digitalWrite(greenLed, HIGH);
//    delayMicroseconds(i);
//    digitalWrite(greenLed, LOW);
//    delayMicroseconds(1000 - i);
//
    digitalWrite(redLed, LOW);
    delayMicroseconds(i);
    digitalWrite(redLed, HIGH);
    delayMicroseconds(1000 - i);
  }
  for(int i = 1000; i > 0; i--){
    digitalWrite(blueLed, HIGH);
    delayMicroseconds(i);
    digitalWrite(blueLed, LOW);
    delayMicroseconds(1000 - i);
    
//    digitalWrite(greenLed, HIGH);
//    delayMicroseconds(i);
//    digitalWrite(greenLed, LOW);
//    delayMicroseconds(1000 - i);
//
    digitalWrite(redLed, LOW);
    delayMicroseconds(i);
    digitalWrite(redLed, HIGH);
    delayMicroseconds(1000 - i);
  }
  
  }
//8i Strobe+
void Strobeplus() {
  interval = 2;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (plusWay==0) {
      Red();
    } else if(plusWay==1) {
      Off();
    } else if(plusWay==2) {
      Blue();
    } else if(plusWay==3) {
      Off();
    } 
    plusWay++;
    if(plusWay>3){
      plusWay = 0;
    }
  }
}
//9j RGBStrobe
void RGBStrobe() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (threeWay==0) {
      Red();
    } else if(threeWay==1) {
      Green();
    } else if(threeWay==2) {
      Blue();
    } 
    threeWay++;
    if(threeWay>2){
      threeWay = 0;
    }
  }
}
//10k Rainbow
void Rainbow() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (rainbowWay==0) {
      Red();
    } else if(rainbowWay==1) {
      Green();
    } else if(rainbowWay==2) {
      Blue();
    } else if(rainbowWay==3) {
      Cyan();
    } else if(rainbowWay==4) {
      Yellow();
    } else if(rainbowWay==5) {
      Magenta();
    } 
    rainbowWay++;
    if(rainbowWay>5){
      rainbowWay = 0;
    }
  }
}
//11L Halfstrobe
void Halfstrobe() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      Red();
      ledState = HIGH;
    } else {
      Blue();
      ledState = LOW;
    }
  }
}
//12m BGStrobe
void BGStrobe() {
 unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      Blue();
      ledState = HIGH;
    } else {
      Green();
      ledState = LOW;
    }
  }
}
//13n GRStrobe
void GRStrobe() {
 unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      Green();
      ledState = HIGH;
    } else {
      Red();
      ledState = LOW;
    }
  }
}
//14oOff
void Off() {
  digitalWrite(greenLed, LOW);
  digitalWrite(blueLed, LOW);
  digitalWrite(redLed, LOW);
}
//15p Next
void Next() {
interval-=20;
  if(interval < 5){
    interval = 5;
  }
}
//16q Demo
void Demo() {
Off();

}
//17r Previous
void Previous() {
  interval+=20;
  if(interval > 500){
    interval = 500;
  }
}

void Extra1() {
  flashy = true;
}
void Extra2() {

}
void Extra3() {

}
void Extra4() {

}
void Extra5() {
  flashy = false;
}









