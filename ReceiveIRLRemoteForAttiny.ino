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

#define pinLed 3

boolean ready = false;
void setup()
{
//  Serial.begin(115200);
//  Serial.println(F("Startup"));
//GIMSK = 0b00100000;    // turns on pin change interrupts
//    PCMSK = 0b00010011;    // turn on interrupts on pins PB0, PB1, &amp;amp; PB4
//    sei();
  // Set LED to output
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, HIGH);
delay(500);
digitalWrite(pinLed, LOW);
  // Start reading the remote. PinInterrupt or PinChangeInterrupt* will automatically be selected
//  if (!IRLremote.begin(pinIR)){
  if (!IRLremote.begin(pinIR))
{
}
    }

void loop()
{
  
  if(ready){
    digitalWrite(pinLed, HIGH);
  }
  // Check if we are currently receiving data
  //if (!IRLremote.receiving()) {
    // Run code that disables interrupts, such as some led strips
  //}

  // Check if new IR protocol data is available
  if (IRLremote.available())
  {
    ready = true;
    // Light Led
    digitalWrite(pinLed, HIGH);

    // Get the new data from the remote
    auto data = IRLremote.read();

    // Print the protocol data
//    Serial.print(F("Address: 0x"));
//    Serial.println(data.address, HEX);
//    Serial.print(F("Command: 0x"));
//    Serial.println(data.command, HEX);
//    Serial.println();

    // Turn Led off after printing the data
    digitalWrite(pinLed, LOW);
    ready = true;
  }
  
  
}
