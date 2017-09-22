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
int delayTime = 250;

int selection = 0;
int maxSelection = 3;

////////////////////////HEX codes:///////////////////////////
uint8_t greenHEX = 0x9;
uint8_t redHEX = 0x8;
uint8_t blueHEX = 0xA;
uint8_t yellowHEX = 0xB;
uint8_t cyanHEX = 0xD;
uint8_t magentaHEX = 0xC;
uint8_t whiteHEX = 0xE;
uint8_t fadeHEX = 0xF;
uint8_t strobeplusHEX = 0x15;
uint8_t RGBStrobeHEX = 0x14;
uint8_t rainbowHEX = 0x16;
uint8_t halfstrobeHEX = 0x17;
uint8_t BGStrobeHEX = 0x19;
uint8_t GRStrobeHEX = 0x18;
uint8_t onHEX = 0x6;
uint8_t offHEX = 0x7;
uint8_t nextHEX = 0x4;
uint8_t previousHEX = 0x5;
uint8_t demoHEX = 0x1A;
uint8_t extraHEX1 = 0x1B;
uint8_t extraHEX2 = 0x11;
uint8_t extraHEX3 = 0x10;
uint8_t extraHEX4 = 0x12;
uint8_t extraHEX5 = 0x13;


void setup()
{
  pinMode(blueLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  //  Serial.begin(115200);
  //  Serial.println(F("Startup"));
  //GIMSK = 0b00100000;    // turns on pin change interrupts
  //    PCMSK = 0b00010011;    // turn on interrupts on pins PB0, PB1, &amp;amp; PB4
  //    sei();
  // Set LED to output
  //  pinMode(pinLed, OUTPUT);
  //  digitalWrite(pinLed, HIGH);
  Red();
  delay(delayTime);
  Off();
  delay(delayTime);
  Green();
  delay(delayTime);
  Off();
  delay(delayTime);
  Blue();
  delay(delayTime);
  Off();
  delay(delayTime);
  //digitalWrite(pinLed, LOW);
  // Start reading the remote. PinInterrupt or PinChangeInterrupt* will automatically be selected
  //  if (!IRLremote.begin(pinIR)){
  if (!IRLremote.begin(pinIR))
  {
  }
}

void loop()
{


  if (ready) {
    Off();
    //not using?
    selection++;
    if (selection >= maxSelection) {
      selection = 0;
    }
  }
  /*
    switch(selection){
       case 0: {
         Red();
       }
       case 1: {
         Green();
       }
       case 2: {
         Blue();
       }
       default:{
         Off(); //never gets here
       }
    }
  */
  // Check if we are currently receiving data
  //if (!IRLremote.receiving()) {
  // Run code that disables interrupts, such as some led strips
  //}

  // Check if new IR protocol data is available
  if (IRLremote.available())
  {
    //    Off();
    //    ready = true;
    // Light Led
    //    digitalWrite(pinLed, HIGH);

    // Get the new data from the remote
    auto data = IRLremote.read();

    // Print the protocol data
    //    Serial.print(F("Address: 0x"));
    //    Serial.println(data.address, HEX);
    //    Serial.print(F("Command: 0x"));
    //    Serial.println(data.command, HEX);
    //    Serial.println();

    // Turn Led off after printing the data
    //    digitalWrite(pinLed, LOW);
    testCommand(data.command);

  }
}

void testCommand(uint8_t inSignal) {
  if (inSignal == greenHEX) { //Green
    Off();
    Blue();
  } else if (inSignal == redHEX) { //Red
    Off();
    Red();
  } else if (inSignal == blueHEX) { //Blue //need to decode letters 15 not working...      Off();
    Green();
  }
}

//0a
void Red() {
  Off();
  digitalWrite(redLed, HIGH);
  
}
//1b green
void Green() {
  Off();
  digitalWrite(greenLed, HIGH);
 
}
//2c blue
void Blue() {
  Off();
  digitalWrite(blueLed, HIGH);
 
}
//3d Yellow
void Yellow() {
  Off();

 
}
//4e Cyan
void Cyan() {
  Off();

 
}
//5f Magenta
void Magenta() {
  Off();


}
//6g White
void White() {

 
}
//7h Fade
void Fade() {

 
}
//8i Strobe+
void Strobeplus() {

 
}
//9j RGBStrobe
void RGBStrobe() {

  
}
//10k Rainbow
void Rainbow() {

  
}
//11L Halfstrobe
void Halfstrobe() {

  
}
//12m BGStrobe
void BGStrobe() {

  
}
//13n GRStrobe
void GRStrobe() {

  
}
//14oOff
void Off() {
  digitalWrite(greenLed, LOW);
  digitalWrite(blueLed, LOW);
  digitalWrite(redLed, LOW);

  
}
//15p Next
void Next() {

  
}
//16q Demo
void Demo() {

  
}
//17r Previous
void Previous() {

  
}










