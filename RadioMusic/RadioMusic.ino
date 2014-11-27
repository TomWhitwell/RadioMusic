/*
RADIO MUSIC 
 https://github.com/TomWhitwell/RadioMusic
 
 Audio out: Onboard DAC, teensy3.1 pin A14/DAC
 
 Bank Button: 2
 Bank LEDs 3,4,5,6
 Reset Button: 8  
 Reset LED 11 
 Reset CV input: 9 
 Channel Pot: A9 
 Channel CV: A8 // check 
 Time Pot: A7 
 Time CV: A6 // check 
 SD Card Connections: 
 SCLK 14
 MISO 12
 MOSI 7 
 SS   10 
 
 NB: Compile using modified versions of: 
 SD.cpp (found in the main Arduino package) 
 play_sd_raw.cpp  - In Teensy Audio Library 
 play_sc_raw.h    - In Teensy Audio Library 
 
 from:https://github.com/TomWhitwell/RadioMusic/tree/master/Collateral/Edited%20teensy%20files
 
 */

#include <EEPROM.h>
#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// Debug /  Modes
boolean DEBUG = false; // Must be true if any verbose options are true 
boolean V1 = false; // Verbose 1 = Print pot positions and calculations for channel selection 
boolean V2 = false; // Verbose 2 = Print activity during startup cycle 
boolean V3 = false; // Verbose 3 = Print activity about hot swap system  


// Options 
boolean MUTE = false; // Softens clicks when changing channel / position, at cost of speed. Fade speed is set by DECLICK 
#define DECLICK 15 // milliseconds of fade in/out on switching 


// GUItool: begin automatically generated code
AudioPlaySdRaw           playRaw1;       //xy=131,81
AudioEffectFade          fade1;          //xy=257,169
AudioAnalyzePeak         peak1;          //xy=317,123
AudioOutputAnalog        dac1;           //xy=334,98
AudioConnection          patchCord1(playRaw1, fade1);
AudioConnection          patchCord2(fade1, dac1);
AudioConnection          patchCord3(playRaw1, peak1);
// GUItool: end automatically generated code

// REBOOT CODES 
#define RESTART_ADDR       0xE000ED0C
#define READ_RESTART()     (*(volatile uint32_t *)RESTART_ADDR)
#define WRITE_RESTART(val) ((*(volatile uint32_t *)RESTART_ADDR) = (val))

// SETUP VARS TO STORE DETAILS OF FILES ON THE SD CARD 
#define MAX_FILES 75
#define BANKS 16
int ACTIVE_BANKS; 
String FILE_TYPE = "RAW";
String FILE_NAMES [BANKS][MAX_FILES];
String FILE_DIRECTORIES[BANKS][MAX_FILES];
unsigned long FILE_SIZES[BANKS][MAX_FILES];
int FILE_COUNT[BANKS];
String CURRENT_DIRECTORY = "0";
File root;
#define BLOCK_SIZE 2 // size of blocks to read - must be more than 1, performance might improve with 16?

// SETUP VARS TO STORE CONTROLS 
#define CHAN_POT_PIN A9 // pin for Channel pot
#define CHAN_CV_PIN A6 // pin for Channel CV 
#define TIME_POT_PIN A7 // pin for Time pot
#define TIME_CV_PIN A8 // pin for Time CV
#define RESET_BUTTON 8 // Reset button 
#define RESET_LED 11 // Reset LED indicator 
#define RESET_CV 9 // Reset pulse input 
boolean CHAN_CHANGED = true; 
boolean RESET_CHANGED = false; 
int timePotOld;
Bounce resetSwitch = Bounce( RESET_BUTTON, 20 ); // Bounce setup for Reset
Bounce resetCv = Bounce( RESET_CV, 10 ); // Bounce setup for Reset

unsigned long CHAN_CHANGED_TIME; 
int PLAY_CHANNEL; 
unsigned long playhead;
char* charFilename;

// BANK SWITCHER SETUP 
#define BANK_BUTTON 2 // Bank Button 
#define LED0 6
#define LED1 5
#define LED2 4
#define LED3 3
Bounce bankSwitch = Bounce( BANK_BUTTON, 20 ); 
int PLAY_BANK = 0; 
#define BANK_SAVE 0

// CHANGE HOW INTERFACE REACTS 
#define HYSTERESIS 5 // MINIMUM MILLIS BETWEEN CHANGES
#define TIME_HYSTERESIS 3 // MINIMUM KNOB POSITIONS MOVED 
#define FLASHTIME 10 // How long do LEDs flash for? 
#define HOLDTIME 400 // How many millis to hold a button to get 2ndary function? 
elapsedMillis showDisplay; // elapsedMillis is a special variable in Teensy - increments every millisecond 
int showFreq = 250; // how many millis between serial Debug updates 
elapsedMillis resetLedTimer = 0;
elapsedMillis bankTimer = 0;
elapsedMillis checkI = 0; // check interface 
int checkFreq = 5; // how often to check the interface in Millis 

// CONTROL THE PEAK METER DISPLAY 
elapsedMillis meterDisplay; // Counter to hide MeterDisplay after bank change 
#define meterHIDE 2000 // how long to hide Meter after bank change 
elapsedMillis fps; // COUNTER FOR PEAK METER FRAMERATE 
#define peakFPS 24 //  FRAMERATE FOR PEAK METER 

void setup() {

  //PINS FOR BANK SWITCH AND LEDS 
  pinMode(BANK_BUTTON,INPUT);
  pinMode(RESET_BUTTON, INPUT);
  pinMode(RESET_CV, INPUT);
  pinMode(RESET_LED, OUTPUT);
  pinMode(LED0,OUTPUT);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  ledWrite(PLAY_BANK);

  // START SERIAL MONITOR   
  if (DEBUG)  {
    Serial.begin(38400); 
    delay(1000);
  };
  if (DEBUG && V2)  Serial.println("Starting up...");

  // MEMORY REQUIRED FOR AUDIOCONNECTIONS   
  AudioMemory(5);
  if (DEBUG && V2)    Serial.println("Set memory...");
  // SD CARD SETTINGS FOR AUDIO SHIELD 
  SPI.setMOSI(7);
  SPI.setSCK(14);
  if (DEBUG && V2)    Serial.println("SD card setting ...");

  // OPEN SD CARD 
  if (!(SD.begin(10))) {
    while (!(SD.begin(10))) {
      if (DEBUG && V2)        Serial.println("Unable to access the SD card");
      ledWrite(15);
      delay(100);
      ledWrite(0);
      delay(100);
    }
  }
  if (DEBUG && V2)    Serial.println("SD card is OK ...");


  // OPEN SD CARD AND SCAN FILES INTO DIRECTORY ARRAYS 
  root = SD.open("/");  
  if (DEBUG && V2)    Serial.println("Open Root ...");
  scanDirectory(root, 0);
  if (DEBUG && V2)    Serial.println("Scan directories ...");
  if (DEBUG && V2)    printFileList();

  // CHECK  FOR SAVED BANK POSITION 
  int a = 0;
a = EEPROM.read(BANK_SAVE);
  if (a >= 0 && a <= ACTIVE_BANKS){
    PLAY_BANK = a;
    CHAN_CHANGED = true;
  }
  else {
EEPROM.write(BANK_SAVE,0);
  };
}



////////////////////////////////////////////////////
///////////////MAIN LOOP//////////////////////////
////////////////////////////////////////////////////

void loop() {



  //////////////////////////////////////////
  // IF FILE ENDS, RESTART FROM THE BEGINNING 
  //////////////////////////////////////////

  if (!playRaw1.isPlaying()){
    playhead = 0;
    RESET_CHANGED = true;
    if (DEBUG && V3)Serial.println("*File Ended*");
  }

  if (playRaw1.failed){
    if (DEBUG && V3) Serial.print("*****EXPLODE****");
    reBoot();
  }


  //////////////////////////////////////////
  ////////REACT TO ANY CHANGES /////////////
  //////////////////////////////////////////


  if (CHAN_CHANGED || RESET_CHANGED){
    if (DEBUG && V3){
      Serial.print("In change block, Offset = ");
      Serial.print(playRaw1.fileOffset());
      Serial.print (" Playhead =");
      Serial.print (playhead);
      Serial.print (" Reset =");
      Serial.print (RESET_CHANGED);
      Serial.print (" CHANNEL =");
      Serial.print (CHAN_CHANGED);
      Serial.print (" failed=");
      Serial.println(playRaw1.failed);
    }
    if (MUTE){  
      fade1.fadeOut(DECLICK);      // fade out before change 
      delay(DECLICK);
    }

    charFilename = buildPath(PLAY_BANK,PLAY_CHANNEL);

    if (RESET_CHANGED == false) playhead = playRaw1.fileOffset(); // Carry on from previous position, unless reset pressed
    playhead = (playhead / 16) * 16; // scale playhead to 16 step chunks 
    playRaw1.playFrom(charFilename,playhead);   // change audio
    delay(10);
    if (DEBUG && V3){
      Serial.print("*File Started:");
      Serial.println(playhead);
    }

    if (MUTE)    fade1.fadeIn(DECLICK);                          // fade back in 
    ledWrite(PLAY_BANK);
    CHAN_CHANGED = false;
    RESET_CHANGED = false; 
    resetLedTimer = 0; // turn on Reset LED 
  }



  //////////////////////////////////////////
  // CHECK INTERFACE  & UPDATE DISPLAYS/////  
  //////////////////////////////////////////

  if (checkI > checkFreq){
    checkInterface(); 
    checkI = 0;  
  }

  if (DEBUG && showDisplay > showFreq){
    //    playDisplay();
    //    whatsPlaying();
    if (DEBUG && V3)Serial.print("In main loop, Offset= ");
    if (DEBUG && V3)Serial.print(playRaw1.fileOffset());
    if (DEBUG && V3)Serial.print(" playTime=");

    showDisplay = 0;
  }

  digitalWrite(RESET_LED, resetLedTimer < FLASHTIME); // flash reset LED 

  if (fps > 1000/peakFPS && meterDisplay > meterHIDE) peakMeter();    // CALL PEAK METER   


}


////////////////////////////////////////////////////
// /////////////FUNCTIONS//////////////////////////
////////////////////////////////////////////////////



//////////////////////////////////////////
// CHECK INTERFACE POTS BUTTONS ETC //////
/////////////////////////////////////////

void checkInterface(){
  unsigned long elapsed;
  // Channel Pot 
  if (DEBUG && V1) Serial.print("Channel pot raw=");
  if (DEBUG && V1)  Serial.print(analogRead(CHAN_POT_PIN));
  if (DEBUG && V1)  Serial.print(" Channel CV raw=");
  if (DEBUG && V1)  Serial.print(analogRead(CHAN_CV_PIN));
  int channel = analogRead(CHAN_POT_PIN) + analogRead(CHAN_CV_PIN);
  if (DEBUG && V1)   Serial.print(" Combined raw=");
  if (DEBUG && V1)  Serial.print(channel); 
  channel = constrain(channel, 0, 1023);
  if (DEBUG && V1)   Serial.print(" constrained=");
  if (DEBUG && V1)  Serial.print(channel); 
  channel = map(channel,0,1024,0,FILE_COUNT[PLAY_BANK]); // Highest pot value = 1 above what's possible (ie 1023+1) and file count is one above the number of the last file (zero indexed)  
  if (DEBUG && V1) Serial.print(" file count=");
  if (DEBUG && V1) Serial.print(FILE_COUNT[PLAY_BANK]);
  if (DEBUG && V1) Serial.print(" mapped=");
  if (DEBUG && V1) Serial.println(channel); 
  elapsed = millis() - CHAN_CHANGED_TIME;
  if (channel != PLAY_CHANNEL && elapsed > HYSTERESIS) {
    PLAY_CHANNEL = channel;
    CHAN_CHANGED = true;
    CHAN_CHANGED_TIME = millis();
  }
  // Time pot & CV 
  int averages = 5; // how many readings to take, to get average
  int timePot = 0;
  for(int a = 0; a < averages; a++){
    timePot += analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN);
  }
  timePot = timePot / averages; 
  timePot = (timePot / 2)*2; 
  timePot = constrain(timePot, 0, 1023); 
  elapsed = millis() - CHAN_CHANGED_TIME;

  if (abs(timePot - timePotOld) > TIME_HYSTERESIS && elapsed > HYSTERESIS){
    unsigned long fileLength = FILE_SIZES[PLAY_BANK][PLAY_CHANNEL];
    unsigned long newTime = ((fileLength/1023) * timePot);
    unsigned long playPosition = playRaw1.fileOffset();
    unsigned long fileStart = (playPosition / fileLength) * fileLength;
    playhead = fileStart + newTime;
    //RESET_CHANGED = true; // THIS LINE = POT CHANGES IMMEDIATELY CAUSE RESET 
    timePotOld = timePot;
  }


  // Reset Button 
  if ( resetSwitch.update() ) {
    RESET_CHANGED = resetSwitch.read();
  }

  // Reset CV 
  if ( resetCv.update() ) RESET_CHANGED = resetCv.read();

  // Hold Reset Button to Change Bank 
  bankTimer = bankTimer * digitalRead(RESET_BUTTON);
  if (bankTimer > HOLDTIME){
    PLAY_BANK++;
    if (PLAY_BANK > ACTIVE_BANKS) PLAY_BANK = 0;   
    CHAN_CHANGED = true;
    bankTimer = 0;  
    meterDisplay = 0;
EEPROM.write(BANK_SAVE, PLAY_BANK);
  }

  // Bank Button - if separate switch installed 
  if ( bankSwitch.update() ) {
    if ( bankSwitch.read() == HIGH ) {
      PLAY_BANK++;
      if (PLAY_BANK >= BANKS) PLAY_BANK = 0; 
      CHAN_CHANGED = true;
EEPROM.write(BANK_SAVE, PLAY_BANK);
    }    
  }
}

//////////////////////////////////////////
// SCAN SD DIRECTORIES INTO ARRAYS //////
/////////////////////////////////////////
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
      if (intCurrentDirectory > ACTIVE_BANKS) ACTIVE_BANKS = intCurrentDirectory; 
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


// TAKE BANK AND CHANNEL AND RETURN PROPERLY FORMATTED PATH TO THE FILE 
char* buildPath (int bank, int channel){
  String liveFilename = bank;
  liveFilename += "/";
  liveFilename += FILE_NAMES[bank][channel];
  char filename[18];
  liveFilename.toCharArray(filename, 17);
  return filename;
}

void reBoot(){
  delay (500);
  WRITE_RESTART(0x5FA0004);
}




// DISPLAY PEAK METER IN LEDS 
void peakMeter(){
  if (peak1.available()) {
    fps = 0;
    int monoPeak = peak1.read() * 5.0;
    ledWrite((pow(2,monoPeak))-1); // 
  }
}


// WRITE A 4 DIGIT BINARY NUMBER TO LED0-LED3 
void ledWrite(int n){
  digitalWrite(LED0, HIGH && (n & B00001000));
  digitalWrite(LED1, HIGH && (n & B00000100));
  digitalWrite(LED2, HIGH && (n & B00000010));
  digitalWrite(LED3, HIGH && (n & B00000001)); 
}




//////////////////////////////////////////////
////////VARIOUS DEBUG DISPLAY FUNCTIONS //////
//////////////////////////////////////////////

// SHOW VISUAL INDICATOR OF TRACK PROGRESS IN SERIAL MONITOR 
void playDisplay(){
  int position = (playRaw1.fileOffset() %  FILE_SIZES[PLAY_BANK][PLAY_CHANNEL]) >> 21;
  int size = FILE_SIZES[PLAY_BANK][PLAY_CHANNEL] >> 21;
  for (int i = 0; i < size; i++){
    if (i == position) Serial.print("|");
    else Serial.print("_");
  }
  Serial.println("");
}


// SHOW CURRENTLY PLAYING TRACK IN SERIAL MONITOR 
void whatsPlaying (){
  Serial.print("Bank:");
  Serial.print(PLAY_BANK);
  Serial.print(" Channel:");
  Serial.print(PLAY_CHANNEL);  
  Serial.print(" File:");  
  Serial.println (charFilename); 
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
      Serial.print (i);
      Serial.print (") ");
      Serial.print (FILE_DIRECTORIES[x][i]);
      Serial.print(" | ");
      Serial.print(FILE_NAMES[x][i]);
      Serial.print(" | ");
      Serial.print(FILE_SIZES[x][i]);
      Serial.println(" | ");
    }
  }
}










