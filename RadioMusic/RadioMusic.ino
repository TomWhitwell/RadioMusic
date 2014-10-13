


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// GUItool: begin automatically generated code
AudioPlaySdRaw           playRaw1;       //xy=183,98
AudioOutputAnalog        dac1;           //xy=334,98
AudioConnection          patchCord1(playRaw1, dac1);
// GUItool: end automatically generated code




void setup() {
  
// START SERIAL MONITOR   
  Serial.begin(9600);
  
// MEMORY REQUIRED FOR AUDIOCONNECTIONS   
  AudioMemory(5);

// SD CARD SETTINGS FOR AUDIO SHIELD 
  SPI.setMOSI(7);
  SPI.setSCK(14);

// REPORT ERROR IF SD CARD CANNOT BE READ 
  if (!(SD.begin(10))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }



}

void loop() {

playRaw1.playFrom("R2.RAW",0);
delay(10000);

  
}
