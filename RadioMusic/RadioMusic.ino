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

// SETUP VARS TO STORE DETAILS OF FILES ON THE SD CARD 
#define MAX_FILES 50
#define BANKS 4
String FILE_TYPE = "WAV";
String FILE_NAMES [BANKS][MAX_FILES];
String FILE_DIRECTORIES[BANKS][MAX_FILES];
unsigned long FILE_SIZES[BANKS][MAX_FILES];
int FILE_COUNT[BANKS];
String CURRENT_DIRECTORY = "0";
File root;
// SETUP VARS TO STORE PLAYBACK AND SPOT CHANGES 
#define POT_MOVE_HYSTERESIS 4 // STEPS BETWEEN CHANGES 
#define POT_TIME_HYSTERESIS 100 // MILLIS BETWEEN CHANGES
int POT_OLD;
unsigned long POT_LAST_MOVE;


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

// OPEN SD CARD AND SCAN FILES INTO DIRECTORY ARRAYS 
root = SD.open("/");  
scanDirectory(root, 0);
  
}





void loop() {

int pot = ReadAndAverage(2,64);

int potChange = pot - POT_OLD;
unsigned long sinceLastPot = millis() - POT_LAST_MOVE;

if (abs(potChange) > POT_MOVE_HYSTERESIS && sinceLastPot > POT_TIME_HYSTERESIS){
  // Change the channel. 
 

  POT_OLD = pot;
  POT_LAST_MOVE = millis();
}





  
}







// READ AVERAGE VALUES FROM NAMED ANALOG INPUT 
int ReadAndAverage (int input_number, int average_count){
  int values;
  for(int i = 0; i < average_count; i++){
   values = values + analogRead(input_number); 
  }
  values = values / average_count;
  return values;
}


// SCAN SD DIRECTORIES INTO ARRAYS 
void scanDirectory(File dir, int numTabs) {
  while(true) {  
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    String fileName = entry.name();

    if (fileName.endsWith(FILE_TYPE) && fileName.startsWith("_") == 0){
      int intCurrentDirectory = CURRENT_DIRECTORY.toInt();
      FILE_NAMES[intCurrentDirectory][FILE_COUNT[intCurrentDirectory]] = entry.name();
      FILE_SIZES[intCurrentDirectory][FILE_COUNT[intCurrentDirectory]] = entry.size();
      FILE_DIRECTORIES[intCurrentDirectory][FILE_COUNT[intCurrentDirectory]] = CURRENT_DIRECTORY;
      FILE_COUNT[intCurrentDirectory]++;

    };

    // Ignore OSX Spotlight and Trash Directories 
    if (entry.isDirectory() && fileName.startsWith("SPOTL") == 0 && fileName.startsWith("TRASH") == 0) {
      CURRENT_DIRECTORY = entry.name();
      scanDirectory(entry, numTabs+1);
    } 
    entry.close();
  }
}

