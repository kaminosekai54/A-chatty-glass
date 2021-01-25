#include "math.h"
// Bullshit for initialising the mp3 card

// Specifically for use with the Adafruit Feather, the pins are pre-set here!

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

// These are the pins used
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)

// Feather ESP8266
#if defined(ESP8266)
  #define VS1053_CS      16     // VS1053 chip select pin (output)
  #define VS1053_DCS     15     // VS1053 Data/command select pin (output)
  #define CARDCS          2     // Card chip select pin
  #define VS1053_DREQ     0     // VS1053 Data request, ideally an Interrupt pin

// Feather ESP32
#elif defined(ESP32)
  #define VS1053_CS      32     // VS1053 chip select pin (output)
  #define VS1053_DCS     33     // VS1053 Data/command select pin (output)
  #define CARDCS         14     // Card chip select pin
  #define VS1053_DREQ    15     // VS1053 Data request, ideally an Interrupt pin

// Feather Teensy3
#elif defined(TEENSYDUINO)
  #define VS1053_CS       3     // VS1053 chip select pin (output)
  #define VS1053_DCS     10     // VS1053 Data/command select pin (output)
  #define CARDCS          8     // Card chip select pin
  #define VS1053_DREQ     4     // VS1053 Data request, ideally an Interrupt pin

// WICED feather
#elif defined(ARDUINO_STM32_FEATHER)
  #define VS1053_CS       PC7     // VS1053 chip select pin (output)
  #define VS1053_DCS      PB4     // VS1053 Data/command select pin (output)
  #define CARDCS          PC5     // Card chip select pin
  #define VS1053_DREQ     PA15    // VS1053 Data request, ideally an Interrupt pin

#elif defined(ARDUINO_NRF52832_FEATHER )
  #define VS1053_CS       30     // VS1053 chip select pin (output)
  #define VS1053_DCS      11     // VS1053 Data/command select pin (output)
  #define CARDCS          27     // Card chip select pin
  #define VS1053_DREQ     31     // VS1053 Data request, ideally an Interrupt pin

// Feather M4, M0, 328, nRF52840 or 32u4
#else
  #define VS1053_CS       6     // VS1053 chip select pin (output)
  #define VS1053_DCS     10     // VS1053 Data/command select pin (output)
  #define CARDCS          5     // Card chip select pin
  // DREQ should be an Int pin *if possible* (not possible on 32u4)
  #define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin

#endif


Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);
  
// defines pins numbers
const int trigPin = A2;
const int echoPin = A5;
// defines variables
long duration;
double distance;
unsigned long previousMillis = 0;
int volume;
int lastVolume = 0;
const double hauteur = 12.4;
const double rayon = 5.95;
void setup() {
pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
pinMode(echoPin, INPUT); // Sets the echoPin as an Input

Serial.begin(115200);

  // if you're using Bluefruit or LoRa/RFM Feather, disable the radio module
  //pinMode(8, INPUT_PULLUP);

  
  if (! musicPlayer.begin()) { // initialise the music player
     while (1);
  }
  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
  
  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(5, 5);
  
#if defined(__AVR_ATmega32U4__) 
  // Timer interrupts are not suggested, better to use DREQ interrupt!
  // but we don't have them on the 32u4 feather...
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int
#else
  // If DREQ is on an interrupt pin we can do background
  // audio playing
//  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
#endif
}


void loop() {
// unsigned long currentMillis = millis();

// Clears the trigPin
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
// Sets the trigPin on HIGH state for 10 micro seconds

digitalWrite(trigPin, HIGH);
delayMicroseconds(10);

digitalWrite(trigPin, LOW);


duration = pulseIn(echoPin, HIGH);
distance= (duration*0.034/2 ) -14.5;
 volume =1000 - ( M_PI * rayon * rayon * distance  );
if (volume >= 0 && volume <= 1000){
Serial.print("Distance: ");
Serial.println(volume);

if (volume >= lastVolume  + 1 || volume <= lastVolume - 1){
findFileName(volume);
lastVolume = volume;
delay(2000);
 }
}
}


void findFileName(int  volume){
  int samples[4];
  //sizeof(String(volume)) / sizeof(String(volume)[0]) + 1];
   int len = 4;
   //sizeof(String(volume)) / sizeof(String(volume)[0]);
   int index = 0;
   int tmpVolume = volume;
   while (len > 0){
    if (len == 4){
if (tmpVolume < 1000){
        samples[index] = 0;
      index ++;
len --;      
        } else {
      samples[index] = ((tmpVolume / 1000) * 1000);
      index ++;
      tmpVolume = tmpVolume - (tmpVolume / 1000) * 1000;
      len --;
      }}

      // if 100 only
if (len == 3){
if (tmpVolume < 100){
        samples[index] = 0;
      index ++;
      len --;
        } else {
      samples[index] =((tmpVolume/ 100) * 100); 
      index ++;
      tmpVolume =tmpVolume - (tmpVolume / 100) * 100;
      len --;
      }}

      // 10 only
      
      if (len == 2){
        if (tmpVolume < 10){
        samples[index] = 0;
      index ++;
      len --;
        } else {
          if (tmpVolume >0 && tmpVolume < 20){
samples[index] = tmpVolume; 
samples[index + 1] = 0;
index = index +2;
      len = 0;
            } else {
      samples[index] = ((tmpVolume / 10) * 10);
      index ++;
      tmpVolume = tmpVolume - (tmpVolume / 10) * 10;
      len --;
    }}}


      // unit only

      if (len == 1){
if (tmpVolume <= 0){      
        samples[index] = 0;
      index ++;
      len --;
        } else {
      samples[index] = tmpVolume;
      index ++;
      tmpVolume = 0;
        len = 0;
      }}
    }
//samples[index] = "nofile";
speak(samples);
    
  }
  
void speak(int* fileName){
  for (int i = 0; i < 5; i++) {
    // if (!fileName[i].equals("nofile")){
  
  String name = "/" + String(fileName[i]) + ".mp3";
  char tmp[sizeof(name)];
    name.toCharArray(tmp, sizeof(tmp));
  // if (musicPlayer.playingMusic){}
  musicPlayer.playFullFile(tmp);
    }  // }
}
