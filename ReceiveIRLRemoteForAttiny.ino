 
/*
   LED code by Tom Hastings
   upgrading k8 juggling clubs internal code
   run on attiny85 @8mhz internal clock
   led's switched via transistors

*/

/*
 * list of signals:

Red:
  Red

Green:
  Green

Blue:
  Blue

Yellow:
  Yellow

Cyan:
  Cyan

Magenta:
  Magenta

White:
  White

Fade:
  Fade some colours (incomplete)

Strobeplus:
  Colour1 - Off - Colour2 - Off (Strobing selection ++)

RGBStrobe:
  RGB

Rainbow:
  RGBCYM

Halfstrobe:
  Blue - Red

BGStrobe:
  Blue - Green

GRStrobe:
  Green - Red

On:
  start recording

Off:
  start playback

Previous:
  Strobe speed --

Next:
  Strobe speed ++

Demo:
  ...............


Extra1:
  Strobing On/Off

Extra2:
  Cyan - Magenta

Extra3:
  Yellow - Magenta

Extra4:
  Yellow - Cyan

Extra5:
  Off

 */

/*
  Copyright (c) 2014-2015 NicoHood
  See the readme for credit to other people.

  IRL Receive

  Receives IR signals from different protocols and prints them to the //Serial monitor.
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
// In order to get this to work had to make changes to: /home/tom/.arduino15/packages/attiny/hardware/avr/1.0.2/variants/tiny8/pins_arduino.h
// notes in the source, added the following line:
// added below myself (TOM) - helps compile IRLRemote code (but how does it work??????)
//#define digitalPinToInterrupt(p) ((p) == 2 ? 0 : ((p) == 3 ? 1 : NOT_AN_INTERRUPT))
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// include PinChangeInterrupt library* BEFORE IRLremote to access more pins if needed
#include "PinChangeInterrupt.h"
//
#include <EEPROM.h>
#include "IRLremote.h"

// Choose a valid PinInterrupt or PinChangeInterrupt* pin of your Arduino board
#define pinIR 4 // this is actually 4 on K8, does the library support this though? NOOOOOO!!!

// Choose the IR protocol of your remote. See the other example for this.
CNec IRLremote;
// CPanasonic IRLremote;
// CHashIR IRLremote;
//#define IRLremote Sony12

//#define pinLed 3

boolean ready = false;
// these are the exact same pins as K8:
// Blue: 1, Green: 3, Red: 2 on K8. Tested now - correct.

int blueLed = 1;  // 1 on attiny - change to 5 on UNO to avoid //Serial conflict
int greenLed = 3; // middle
int redLed = 2;   // Actually 2 on K8
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

long previousMillis = 0; // will store last time LED was updated
long currentMillis2 = 0; 

long demoInterval = 10000; // change demo every 10 seconds
long interval = 125;       // interval at which to blink (milliseconds) //todo: add this to eeprom memory - save last state
int ledState = LOW;
boolean flashy = false;

volatile uint8_t inSignal = 0xA;   // for storing signal - fade for test use 0xA (blue) for production
volatile uint8_t prevSignal = 0xA; // for memory
volatile uint8_t oneSignal = 0xA;  // for flashy3Way
volatile uint8_t twoSignal = 0xA;  // for flashy3Way
boolean flashThreeWay = false;

uint8_t threeWay = 0;
uint8_t fadeAWay = 0;
uint8_t rainbowWay = 0;
uint8_t plusWay = 0;

long timings[50]; // max should be 128, 127 to be safe, or maybe just 100 to keep some memory for other things... ok no 50 save size for attiny...
uint8_t colours[50];
boolean recording = false;
long recStartTime = 0;
long recEndTime = 1000000;
boolean playing = false;

int eepromTimeAddr = 0;      // incr by 4 for long
int eepromColAddr = 384;     // maximum 128 signals if nothing else is saved...
int maxEepromSignalNum = 50; // using only 50 now
int runNum = 0;

int strobePlusOptions = 0;
int strobePlusOptionsMax = 5;

uint8_t lastPattern = 0;
uint8_t endPattern = 21;

void setup()
{
  // eeprom write test:
  // EEPROMWritelong(0, 5000);
  // EEPROMWritelong(4, 10000);
  //  EEPROM.write(20, yellowHEX);
  pinMode(blueLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  //Serial.begin(115200); // //Serial conflicts with Pin1 - Green I think!
  //Serial.println("Startup");

  // Startup sequence?:
  // Red();
  // delay(500);
  // Green();
  // delay(500);
  // Blue();

  /*
    long timeDelay1 = timings[0];
    delay(timeDelay1);
    Red();
    long timeDelay2 = timings[1];
    delay(timeDelay2);
    Green();
  */

  //   Start reading the remote. PinInterrupt or PinChangeInterrupt* will automatically be selected
  if (!IRLremote.begin(pinIR))
  {
  }

  for (int i = 0; i < maxEepromSignalNum; i++)
  {
    timings[i] = EEPROMReadlong(i * 4);
    //Serial.print("timings ");
    //Serial.print(i);
    //Serial.print (" = ");
    //Serial.println(timings[i]);
    colours[i] = EEPROM.read(384 + i);
    //Serial.print("colours ");
    //Serial.print(i);
    //Serial.print(" = ");
    //Serial.println(colours[i]);
  }

  //get saved value for interval and flashy:
  interval = EEPROMReadlong(500);
  if(interval > 500 || interval < 5)
  {
    interval = 125;
  }
  //Serial.print("Interval: ");
  //Serial.println(interval);

// Red();
// increment patterns on reboot:
// todo: to save eeprom writes, need to re-factor the strobing patterns, as they call red() blue() etc all the time
// this would wear eeprom too fast. For now not saving last pattern on button press. 
  // lastPattern = 5; 
  lastPattern = EEPROM.read(510); //this is not reading correctly. Why? 
  // delay(100); //delay for read? 

  lastPattern++;
  if(lastPattern > endPattern)
  {
    lastPattern = 0;
  }
  //save pattern for next time: 
  EEPROM.write(510, lastPattern);
// delay(100); //write delay? 
  //run pattern (commented some not working/not applicable)
  //test:
  
  switch(lastPattern)
  {
    case 0: 
      inSignal = greenHEX;
      break;
    case 1: 
      inSignal = redHEX;
      break;
    case 2: 
      inSignal = blueHEX;
      break;
    case 3: 
      inSignal = yellowHEX;
      break;
    case 4: 
      inSignal = cyanHEX;
      break;
    case 5: 
      inSignal = magentaHEX;
      break;
    case 6: 
      inSignal = whiteHEX;
      break;
    case 7: 
      inSignal = redHEX;
      // Fade();
      break;
    case 8: 
      inSignal = greenHEX;
      // Demo();
      break;
    case 9: 
      inSignal = blueHEX;
      // Strobeplus();
      break;
    case 10: 
      inSignal = RGBStrobeHEX;
      break;
    case 11: 
      inSignal = rainbowHEX;
      break;
    case 12: 
      inSignal = halfstrobeHEX;
      break;
    case 13: 
      inSignal = BGStrobeHEX;
      break;
    case 14: 
      inSignal = GRStrobeHEX;
      break;
    case 15: 
      inSignal = redHEX;
      // Next();
      break;
    case 16:
      inSignal = greenHEX;
      // Previous();
      break;
    case 17: 
      // Blue();
      // inSignal = extraHEX1; // flashy on - does nothing
      inSignal = blueHEX;
      break;
    case 18: 
      inSignal = extraHEX2;
      break;
    case 19: 
      inSignal = extraHEX3;
      break;
    case 20: 
      inSignal = extraHEX4;
      break;
    case 21: 
      inSignal = extraHEX5;
      // Extra5();
      break;
    //not saving Off() obviously
    // default: 
    //   inSignal = redHEX;
    //   break;
  }

  //test:
  // inSignal = rainbowHEX;
  
  
}

void loop()
{

  // test:
  if (playing)
  {
    if (runNum > maxEepromSignalNum - 1)
    {
      runNum = 0;
    }
    //another sanity check: 
    if(timings[runNum] > 0){   
        // //Serial.print("Checking timing for ");
        // //Serial.print(timings[runNum]);
        // //Serial.println(" timings[runNum");
        currentMillis2 = millis() - recStartTime; // reset the clock! At the beginning of play this should be 0!
        if (currentMillis2 < timings[runNum])
        {
                // //Serial.print("< ");
                // //Serial.println(runNum);
        }
        //   I think currently timings[] array must be in time order to work. If the last time is smaller than the previous nothing happens. todo: sort?
        else if (currentMillis2 >= timings[runNum] && currentMillis2 <= timings[runNum + 1]) //todo: this omits the last signal..
        { // saved signal
                //Serial.print("== ");
                //Serial.println(runNum);
        //sanity check: 
            if (colours[runNum] < 22){ //max is 22 otherwise it's a bogus signal 
                inSignal = colours[runNum];
                //Serial.print("colour is set to: ");
                //Serial.println(colours[runNum]);
            }      
      
      //todo: if inSignal is nothing, go to runNum = 0
        }
    }
    runNum++;
  }

  // Check if new IR protocol data is available:

  if (IRLremote.available())
  {
    auto data = IRLremote.read();
    if (data.command == 0x0)
    {
      // inbetween signal do nothing...
    }
    else
    {
      inSignal = data.command;
      if (recording)
      { // recording activated by pressing 'ON' button      
        if (inSignal == onHEX || inSignal == offHEX) // don't save "RECORD" signal or "PLAY" signal: 
        {

        } else{
            // eeprom write test:
            // Note: need to record strobing/not strobing information as well as strobe speed here too. or just press strobe button...buggy though
            EEPROMWritelong(eepromTimeAddr, millis() - recStartTime); // save this in Array for immediate playback/testing?
            timings[eepromTimeAddr/4] = millis() - recStartTime; //save for immediate playback
            //Serial.print("saved time: ");
            //Serial.print(millis()-recStartTime);
            EEPROM.write(eepromColAddr, inSignal); // and save in Array too?
            colours[eepromColAddr-384] = inSignal; //save for immediate playback
            //Serial.print(" Signal: ");
            //Serial.println(inSignal);
            eepromTimeAddr += 4;
            eepromColAddr++; // on to the next colour
            // change below timer array only saving max 50!
            if (eepromColAddr > 500)
            {                       // too much go back
            //Serial.println("too much record, go back!");
            eepromTimeAddr = 124; // last one again
            eepromColAddr = 496;  // last one again
            }
        }
      }

      if (playing)
      {
        playing = false; //stop playback
      }
    }
  }
  if (flashThreeWay)
  {
    flash3Way();
  }

  if (flashy)
  {
    flash();
  }
  else
  {
    testCommand();
  }
}

void flash()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
    {
      Off();
      ledState = HIGH;
    }
    else
    {
      testCommand();
      ledState = LOW;
    }
  }
}

void flash3Way()
{
  //  interval = 2;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    if (plusWay == 0)
    {
      //      inSignal = offHEX;
      inSignal = oneSignal;
      testCommand();
    }
    else if (plusWay == 1)
    {
      //      Off();
      inSignal = extraHEX5; // off signal
      testCommand();
    }
    else if (plusWay == 2)
    {
      //      inSignal = extraHEX5;
      inSignal = twoSignal;
      testCommand();
    }
    else if (plusWay == 3)
    {
      //      Off();
      inSignal = extraHEX5; // off signal
      testCommand();
    }
    plusWay++;
    if (plusWay > 3)
    {
      plusWay = 0;
    }
  }
}

void testCommand()
{
  if (inSignal == greenHEX)
  { // Green
    Green();
    prevSignal = inSignal;
  }
  else if (inSignal == redHEX)
  { // Red
    Red();
    prevSignal = inSignal;
  }
  else if (inSignal == blueHEX)
  { // Blue
    Blue();
    prevSignal = inSignal;
  }
  else if (inSignal == yellowHEX)
  {
    Yellow();
    prevSignal = inSignal;
  }
  else if (inSignal == cyanHEX)
  {
    Cyan();
    prevSignal = inSignal;
  }
  else if (inSignal == magentaHEX)
  {
    Magenta();
    prevSignal = inSignal;
  }
  else if (inSignal == whiteHEX)
  {
    White();
    prevSignal = inSignal;
  }
  else if (inSignal == fadeHEX)
  {
    flashy = false; // not working in flashy mode
    flashThreeWay = false;
    Fade();
    prevSignal = inSignal;
  }
  else if (inSignal == demoHEX)
  { // demo not defined!
    Demo();
    flashy = false;
    prevSignal = inSignal;
  }
  else if (inSignal == strobeplusHEX)
  {
    // interval = 2; //no rather use current interval! 
    flashy = false; // not working in flashy mode
    flashThreeWay = false;
    Strobeplus();
    // prevSignal = inSignal;
  }
  else if (inSignal == RGBStrobeHEX)
  {
    flashy = false; // not working in flashy mode
    flashThreeWay = false;
    RGBStrobe();
    prevSignal = inSignal;
  }
  else if (inSignal == rainbowHEX)
  {
    flashy = false; // not working in flashy mode
    flashThreeWay = false;
    Rainbow();
    prevSignal = inSignal;
  }
  else if (inSignal == halfstrobeHEX)
  {
    flashy = false; // not working in flashy mode
    flashThreeWay = false;
    Halfstrobe();
    prevSignal = inSignal;
  }
  else if (inSignal == BGStrobeHEX)
  {
    flashy = false; // not working in flashy mode
    flashThreeWay = false;
    BGStrobe();
    prevSignal = inSignal;
  }
  else if (inSignal == GRStrobeHEX)
  {
    flashy = false; // not working in flashy mode
    flashThreeWay = false;
    GRStrobe();
    prevSignal = inSignal;
  }
  else if (inSignal == nextHEX)
  {
    Next();
    inSignal = prevSignal; // now switch to whatever we were doing before .. need to test for non-colour signals here though...
  }
  else if (inSignal == previousHEX)
  {
    Previous();
    inSignal = prevSignal;
  }
  else if (inSignal == extraHEX1)
  {
    Extra1();
    inSignal = prevSignal;
  }
  else if (inSignal == extraHEX2)
  {
    flashy = false;
    flashThreeWay = false;
    Extra2();
    prevSignal = inSignal;
  }
  else if (inSignal == extraHEX3)
  {
    flashy = false;
    flashThreeWay = false;
    Extra3();
    prevSignal = inSignal;
  }
  else if (inSignal == extraHEX4)
  {
    flashy = false;
    flashThreeWay = false;
    Extra4();
    prevSignal = inSignal;
  }
  else if (inSignal == extraHEX5)
  {
    Extra5();
    flashy = false;
    prevSignal = inSignal;
  }
  else if (inSignal == offHEX)
  {             // start playing back recording
    //Serial.println("PLAY START");
    runNum = 0; // reset to start again
    recStartTime = millis();
    recording = false;
    playing = true;
    //    inSignal = offHEX;
    inSignal = prevSignal; // hmmm what does this actually do?
    //    recEndTime = millis();
    //    recording = false;
    Off();
  }
  else if (inSignal == onHEX)
  { // start recording
  //todo: re-set all eeprom
  //indicator for recording: Red() - not working? 
    //Serial.println("RECORD START");
    On();
    inSignal = prevSignal;
    recStartTime = millis();
    playing = false;
    recording = true;
    // Red();
  }
}

// 0a
void Red()
{
  digitalWrite(redLed, HIGH);
  digitalWrite(blueLed, LOW);
  digitalWrite(greenLed, LOW);
}
// 1b green
void Green()
{
  digitalWrite(greenLed, HIGH);
  digitalWrite(blueLed, LOW);
  digitalWrite(redLed, LOW);
}
// 2c blue
void Blue()
{
  digitalWrite(blueLed, HIGH);
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);
}
// 3d Yellow
void Yellow()
{
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, HIGH);
  digitalWrite(blueLed, LOW);
}
// 4e Cyan
void Cyan()
{
  digitalWrite(blueLed, HIGH);
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);
}
// 5f Magenta
void Magenta()
{
  digitalWrite(blueLed, HIGH);
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, LOW);
}
// 6g White
void White()
{
  digitalWrite(blueLed, HIGH);
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, HIGH);
}
// 7h Fade
void Fade()
{
  // after some testing, on attiny85 Red and Blue pins support pwm but green does not. Need another solution for fading:
  // so here it is:
  long fadeSpeed = 500;
  for (int i = 1; i < fadeSpeed; i++)
  {
    digitalWrite(blueLed, HIGH);
    delayMicroseconds(i);
    digitalWrite(blueLed, LOW);
    delayMicroseconds(fadeSpeed - i);

    digitalWrite(greenLed, HIGH);
    delayMicroseconds(i);
    digitalWrite(greenLed, LOW);
    delayMicroseconds(fadeSpeed - i);
  }

  for (int i = 1; i < fadeSpeed; i++)
  {
    digitalWrite(blueLed, LOW);
    delayMicroseconds(i);
    digitalWrite(blueLed, HIGH);
    delayMicroseconds(fadeSpeed - i);

    digitalWrite(greenLed, LOW);
    delayMicroseconds(i);
    digitalWrite(greenLed, HIGH);
    delayMicroseconds(fadeSpeed - i);
  }
  

  for (int i = 1000; i > 0; i--) {
    digitalWrite(greenLed, HIGH);
    delayMicroseconds(1000 - i);
    digitalWrite(greenLed, LOW);
    delayMicroseconds(i);


    //    digitalWrite(greenLed, HIGH);
    //    delayMicroseconds(i);
    //    digitalWrite(greenLed, LOW);
    //    delayMicroseconds(1000 - i);
    //
    digitalWrite(blueLed, HIGH);
    delayMicroseconds(1000 - i);
    digitalWrite(blueLed, LOW);
    delayMicroseconds(i);

  }
  

  
  for (int i = 1000; i > 0; i--) {
    digitalWrite(greenLed, HIGH);
    delayMicroseconds(i);
    digitalWrite(greenLed, LOW);
    delayMicroseconds(1000 - i);

    //    digitalWrite(greenLed, HIGH);
    //    delayMicroseconds(i);
    //    digitalWrite(greenLed, LOW);
    //    delayMicroseconds(1000 - i);
    //
    digitalWrite(blueLed, LOW);
    delayMicroseconds(i);
    digitalWrite(blueLed, HIGH);
    delayMicroseconds(1000 - i);
  }
 
}
// 8i Strobe+
void Strobeplus()
{
  switch (strobePlusOptions)
  {
  case 0:
    oneSignal = redHEX;
    twoSignal = blueHEX;
    break;
  case 1:
    oneSignal = blueHEX;
    twoSignal = greenHEX;
    break;
  case 2:
    oneSignal = greenHEX;
    twoSignal = redHEX;
    break;
  case 3:
    oneSignal = cyanHEX;
    twoSignal = magentaHEX;
    break;
  case 4:
    oneSignal = magentaHEX;
    twoSignal = yellowHEX;
    break;
  case 5:
    oneSignal = yellowHEX;
    twoSignal = cyanHEX;
    break;
  default:
    // nothing
    break;
  }
  strobePlusOptions++;
  if (strobePlusOptions > strobePlusOptionsMax)
  {
    strobePlusOptions = 0;
  }
  flashThreeWay = true;
}
// 9j RGBStrobe
void RGBStrobe()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    if (threeWay == 0)
    {
      Red();
    }
    else if (threeWay == 1)
    {
      Green();
    }
    else if (threeWay == 2)
    {
      Blue();
    }
    threeWay++;
    if (threeWay > 2)
    {
      threeWay = 0;
    }
  }
}
// 10k Rainbow
void Rainbow()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    if (rainbowWay == 0)
    {
      Red();
    }
    else if (rainbowWay == 1)
    {
      Green();
    }
    else if (rainbowWay == 2)
    {
      Blue();
    }
    else if (rainbowWay == 3)
    {
      Cyan();
    }
    else if (rainbowWay == 4)
    {
      Yellow();
    }
    else if (rainbowWay == 5)
    {
      Magenta();
    }
    rainbowWay++;
    if (rainbowWay > 5)
    {
      rainbowWay = 0;
    }
  }
}
// 11L Halfstrobe
void Halfstrobe()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
    {
      Red();
      ledState = HIGH;
    }
    else
    {
      Blue();
      ledState = LOW;
    }
  }
}
// 12m BGStrobe
void BGStrobe()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
    {
      Blue();
      ledState = HIGH;
    }
    else
    {
      Green();
      ledState = LOW;
    }
  }
}
// 13n GRStrobe
void GRStrobe()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
    {
      Green();
      ledState = HIGH;
    }
    else
    {
      Red();
      ledState = LOW;
    }
  }
}
// 14oOff
void Off()
{
  digitalWrite(greenLed, LOW);
  digitalWrite(blueLed, LOW);
  digitalWrite(redLed, LOW);
}
void On()
{
//re-set eeprom recording data to 0: 
// Note: need to record strobing/not strobing information as well as strobe speed here too. or just press strobe button...buggy though
  for (int i = 0; i < maxEepromSignalNum; i++)
  {
    timings[i] = 0; // time
    colours[i] = 0; // colour
    EEPROMWritelong(i*4, 0); // time
    EEPROM.write(i+384, 255);  // colour
  }
            
  eepromTimeAddr = 0; // re-set to first address
  eepromColAddr = 384;  // re-set to first address!
  recording = true;
}
// 15p Next
void Next()
{
  interval -= 20;
  if (interval < 5)
  {
    interval = 5;
  }
  EEPROMWritelong(500, interval); 
}
// 16q Demo
void Demo()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= demoInterval)
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    if (rainbowWay == 0)
    {
      Red();
    }
    else if (rainbowWay == 1)
    {
      Green();
    }
    else if (rainbowWay == 2)
    {
      Blue();
    }
    else if (rainbowWay == 3)
    {
      Cyan();
    }
    else if (rainbowWay == 4)
    {
      Yellow();
    }
    else if (rainbowWay == 5)
    {
      Magenta();
    }
    rainbowWay++;
    if (rainbowWay > 5)
    {
      rainbowWay = 0;
    }
  }
}
// 17r Previous
void Previous()
{
  interval += 20;
  if (interval > 500)
  {
    interval = 500;
  }
  EEPROMWritelong(500, interval);
}

void Extra1()
{
  flashy = !flashy; // toggle strobing
  if(flashThreeWay){
    flashThreeWay = false; // exit flashy three way
    Red();
  }
}
void Extra2()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
    {
      Cyan();
      ledState = HIGH;
    }
    else
    {
      Magenta();
      ledState = LOW;
    }
  }
}
void Extra3()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
    {
      Magenta();
      ledState = HIGH;
    }
    else
    {
      Yellow();
      ledState = LOW;
    }
  }
}
void Extra4()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
    {
      Yellow();
      ledState = HIGH;
    }
    else
    {
      Cyan();
      ledState = LOW;
    }
  }
}
void Extra5()
{
  Off();
}

// This function will write a 4 byte (32bit) long to the eeprom at
// the specified address to address + 3.
void EEPROMWritelong(int address, long value)
{
  // Decomposition from a long to 4 bytes by using bitshift.
  // One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  // Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

// This function will return a 4 byte (32bit) long from the eeprom
// at the specified address to address + 3.
long EEPROMReadlong(long address)
{
  // Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  // Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
