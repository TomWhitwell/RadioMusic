/*
RADIO MUSIC 

Audio out: Onboard DAC, teensy3.1 pin A14/DAC

SD Card Connections: 
SCLK 14
MISO 12
MOSI 7 
SS   10 



*/


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

  int pot = ReadAndAverage(2,64);




  
}




int ReadAndAverage (int input_number, int average_count){
  int values;
  for(int i = 0; i < average_count; i++){
   values = values + analogRead(input_number); 
  }
  values = values / average_count;
  return values;
}

