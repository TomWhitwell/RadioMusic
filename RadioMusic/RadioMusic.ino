/*
RADIO MUSIC 
 
 Audio out: Onboard DAC, teensy3.1 pin A14/DAC
 
 SD Card Connections: 
 SCLK 14
 MISO 12
 MOSI 7 
 SS   10 
 
 
 
 */

#include <Bounce.h>
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
#define MAX_FILES 100
#define BANKS 4
String FILE_TYPE = "RAW";
String FILE_NAMES [BANKS][MAX_FILES];
String FILE_DIRECTORIES[BANKS][MAX_FILES];
unsigned long FILE_SIZES[BANKS][MAX_FILES];
int FILE_COUNT[BANKS];
String CURRENT_DIRECTORY = "0";
File root;

// SETUP VARS TO STORE CONTROLS 
#define CHAN_POT_PIN 9 // pin for Channel pot
#define CHAN_CV_PIN 8 // pin for Channel CV 
int CHAN_POT;
int CHAN_CV;
boolean CHAN_CHANGED = true; 
unsigned long CHAN_CHANGED_TIME; 
int PLAY_CHANNEL; 
unsigned long loopcount; 
unsigned long playhead;
char* charFilename;

// BANK SWITCHER SETUP 
#define BUTTON 2
#define LED0 3
#define LED1 4
#define LED2 5
#define LED3 6
Bounce bankSwitch = Bounce( BUTTON, 20 ); 
int PLAY_BANK = 0; 


// CHANGE HOW INTERFACE REACTS 
#define HYSTERESIS 5 // MINIMUM MILLIS BETWEEN CHANGES



void setup() {

  //PINS FOR BANK SWITCH AND LEDS 
  pinMode(BUTTON,INPUT);
  pinMode(LED0,OUTPUT);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
ledWrite(pow(2,PLAY_BANK));
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
delay(2000);
printFileList();
}





void loop() {

  checkInterface(); 

  if (CHAN_CHANGED == true){
    charFilename = buildPath(PLAY_BANK,PLAY_CHANNEL);
    playhead = playRaw1.fileOffset();
    playRaw1.playFrom(charFilename,playhead);
    ledWrite(pow(2,PLAY_BANK));
    Serial.print("Bank:");
    Serial.print(PLAY_BANK);
    Serial.print(" Channel:");
    Serial.print(PLAY_CHANNEL);  
    Serial.print(" File:");  
    Serial.println (charFilename);
    CHAN_CHANGED = false; 
  }


// IF FILE ENDS, RESTART FROM THE BEGINNING 
if (playRaw1.isPlaying() == false){
CHAN_CHANGED = true; 
}



loopcount++;
}


// WRITE A 4 DIGIT BINARY NUMBER TO LED0-LED3 
void ledWrite(int n){
    digitalWrite(LED0, HIGH && (n & B00001000));
    digitalWrite(LED1, HIGH && (n & B00000100));
    digitalWrite(LED2, HIGH && (n & B00000010));
    digitalWrite(LED3, HIGH && (n & B00000001)); 
}



// READ AND SCALE POTS AND CVs AND RETURN TO GLOBAL VARIBLES 

void checkInterface(){
 
  // Channel Pot 
  int channel = analogRead(CHAN_POT_PIN) + analogRead(CHAN_CV_PIN);
  channel = constrain(channel, 0, 1024);
  channel = map(channel,0,1024,0,FILE_COUNT[PLAY_BANK]);
  unsigned long elapsed = millis() - CHAN_CHANGED_TIME;
  if (channel != PLAY_CHANNEL && elapsed > HYSTERESIS) {
    PLAY_CHANNEL = channel;
    CHAN_CHANGED = true;
  }

// Bank Button 
  if ( bankSwitch.update() ) {
    if ( bankSwitch.read() == HIGH ) {
      PLAY_BANK++;
      if (PLAY_BANK >= BANKS) PLAY_BANK = 0;   
    CHAN_CHANGED = true;
    }    
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
//      ledWrite (FILE_COUNT[intCurrentDirectory]);

    };

    // Ignore OSX Spotlight and Trash Directories 
    if (entry.isDirectory() && fileName.startsWith("SPOTL") == 0 && fileName.startsWith("TRASH") == 0) {
      CURRENT_DIRECTORY = entry.name();
      scanDirectory(entry, numTabs+1);
    } 
    entry.close();
  }
}


// TAKE BANK AND CHANNEL AND RETURN PROPERLY FORMATTED PATH TO THE FILE 
char* buildPath (int bank, int channel){
  String liveFilename = bank;
  liveFilename += "/";
  liveFilename += FILE_NAMES[bank][channel];
  char filename[18];
  liveFilename.toCharArray(filename, 17);
  Serial.println(filename);

  return filename;
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




