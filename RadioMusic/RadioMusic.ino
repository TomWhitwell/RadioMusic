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
String FILE_TYPE = "RAW";
String FILE_NAMES [BANKS][MAX_FILES];
String FILE_DIRECTORIES[BANKS][MAX_FILES];
unsigned long FILE_SIZES[BANKS][MAX_FILES];
int FILE_COUNT[BANKS];
String CURRENT_DIRECTORY = "0";
File root;
// SETUP VARS TO STORE PLAYBACK AND SPOT CHANGES 
#define CHANNELPIN 2 // which analog pin takes the channel cv/knob 
#define POT_MOVE_HYSTERESIS 4 // STEPS BETWEEN CHANGES 
#define POT_TIME_HYSTERESIS 0 // MILLIS BETWEEN CHANGES
int POT_OLD;
unsigned long POT_LAST_MOVE;
int PLAY_BANK = 1; // preset as 1 before bank changing system built 
int PLAY_CHANNEL; 


void setup() {

  // START SERIAL MONITOR   
  Serial.begin(38400);

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

  // Read Channel pot / CV 
int pot = analogRead(CHANNELPIN);

// Check to see if Channel pot has changed a little, or recently 
  int potChange = pot - POT_OLD;
  unsigned long sinceLastPot = millis() - POT_LAST_MOVE;


  if (abs(potChange) > POT_MOVE_HYSTERESIS && sinceLastPot > POT_TIME_HYSTERESIS){
PLAY_CHANNEL = map(pot,0,1024,0,FILE_COUNT[PLAY_BANK]);
  
  String liveFilename = PLAY_BANK;
  liveFilename += "/";
  liveFilename += FILE_NAMES[PLAY_BANK][PLAY_CHANNEL];
  char charFilename[18];
liveFilename.toCharArray(charFilename, 17);
Serial.println(charFilename);

if (playRaw1.isPlaying()){
unsigned long playhead = playRaw1.fileOffset();
playRaw1.playFrom(charFilename,playhead);
}
else {
 playRaw1.playFrom(charFilename,0); 
}

    POT_OLD = pot;
    POT_LAST_MOVE = millis();
  }
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

void printFileList(){

  for (int x = 0; x < BANKS; x++){ 
    Serial.print("Bank: ");
    Serial.println(x);

    Serial.print (FILE_COUNT[x]);
    Serial.print(" ");
    Serial.print (FILE_TYPE);
    Serial.println(" Files found"); 

    for (int i = 0; i < FILE_COUNT[x]; i++){
      Serial.print (FILE_DIRECTORIES[x][i]);
      Serial.print(" | ");
      Serial.print(FILE_NAMES[x][i]);
      Serial.print(" | ");
      Serial.print(FILE_SIZES[x][i]);
      Serial.println(" | ");
    }
  }
}


