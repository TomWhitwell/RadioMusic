#define fwVersion 1
#define fwPoint 3

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
#include <SPI.h>
#include <SD.h>
#include <Wire.h>


// OPTIONS TO READ FROM THE SD CARD, WITH DEFAULT VALUES 
boolean MUTE = false;                // Softens clicks when changing channel / position, at cost of speed. Fade speed is set by DECLICK 
int DECLICK= 15;                      // milliseconds of fade in/out on switching 
boolean ShowMeter = true;            // Does the VU meter appear?  
int meterHIDE = 2000;                // how long to show the meter after bank change in Milliseconds 
boolean ChanPotImmediate = true;     // Settings for Pot / CV response.
boolean ChanCVImmediate = true;      // TRUE means it jumps directly when you move or change.
boolean StartPotImmediate = false;   // FALSE means it only has an effect when RESET is pushed or triggered 
boolean StartCVImmediate = false; 
int StartCVDivider = 2;              // Changes sensitivity of Start control. 1 = most sensitive, 512 = lest sensitive (i.e only two points) 
boolean Looping = true;              // When a file finishes, start again from the beginning 
File settingsFile;




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
#define MAX_FILES 500 // Maximum number of files per bank 
#define MAX_BANKS 32 // Maximum number of banks 
int ACTIVE_BANKS = 0; // How many directories are accessible. Directory structure must start at "0" and be sequential 0, 1, 2, 3 etc 
String FILE_TYPE = "RAW"; // only looking for .raw files 
File root;
String FILE_NAMES[MAX_FILES+1]; // Array to hold the file names in the active bank 
unsigned long FILE_SIZES[MAX_FILES+1]; // Array to hold file sizes in the active bank 
int FILE_COUNT;
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
Bounce resetSwitch = Bounce( RESET_BUTTON, 20 ); // Bounce setup for Reset
int PLAY_CHANNEL; 
int NEXT_CHANNEL; 
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
int chanHyst = 3; // how many steps to move before making a change (out of 1024 steps on a reading) 
int timHyst = 6; 
elapsedMillis chanChanged; 
elapsedMillis timChanged; 
int sampleAverage = 40;
int chanPotOld;
int chanCVOld;
int timPotOld;
int timCVOld;
#define FLASHTIME 10 // How long do LEDs flash for? 
#define HOLDTIME 400 // How many millis to hold a button to get 2ndary function? 
elapsedMillis showDisplay; // elapsedMillis is a special variable in Teensy - increments every millisecond 
int showFreq = 250; // how many millis between serial Debug updates 
elapsedMillis resetLedTimer = 0;
elapsedMillis bankTimer = 0;
elapsedMillis checkI = 0; // check interface 
int checkFreq = 10; // how often to check the interface in Millis 


// CONTROL THE PEAK METER DISPLAY 
elapsedMillis meterDisplay; // Counter to hide MeterDisplay after bank change 
elapsedMillis fps; // COUNTER FOR PEAK METER FRAMERATE 
#define peakFPS 12   //  FRAMERATE FOR PEAK METER 

// OPERATING SYSTEM BLINKER ON STARTUP 
#define tLED 13
#define osFlashRepeats 2
boolean osFlash[50];
int osFlashCount; 
elapsedMillis osFlashClock; 
int osFlashTick; 
int osFlashTimes; 



////////////////////////////////////////////////////
///////////////SETUP////////////////////////////////
////////////////////////////////////////////////////


void setup() {

  //DEFINE PINS FOR BANK SWITCH AND LEDS 
  pinMode(BANK_BUTTON,INPUT);
  pinMode(RESET_BUTTON, INPUT);
  pinMode(RESET_CV, INPUT);
  pinMode(RESET_LED, OUTPUT);
  pinMode(LED0,OUTPUT);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  pinMode(tLED, OUTPUT);  

  ledWrite(PLAY_BANK);

// START SERIAL MONITOR   
Serial.begin(38400); 


  // MEMORY REQUIRED FOR AUDIOCONNECTIONS   
  AudioMemory(5);
  // SD CARD SETTINGS FOR AUDIO SHIELD 
  SPI.setMOSI(7);
  SPI.setSCK(14);

  // OPEN SD CARD 
  int crashCountdown; 
  if (!(SD.begin(10))) {
    while (!(SD.begin(10))) {
      ledWrite(15);
      delay(100);
      ledWrite(0);
      delay(100);
      crashCountdown++;
      if (crashCountdown > 6)     reBoot();

    }
  }

  // READ SETTINGS FROM SD CARD 

  root = SD.open("/");  

  if (SD.exists("settings.txt")) {
    readSDSettings();
  }

  else { 
    writeSDSettings();
  };


  // OPEN SD CARD AND COUNT THE NUMBER OF AVAILABLE BANKS  
  ACTIVE_BANKS = countDirectory();

  // CHECK  FOR SAVED BANK POSITION 
  int a = 0;
  a = EEPROM.read(BANK_SAVE);
  if (a >= 0 && a < ACTIVE_BANKS-1){
    PLAY_BANK = a;
    CHAN_CHANGED = true;
  }
  else {
    EEPROM.write(BANK_SAVE,0);
  };



  FILE_COUNT = loadFiles(PLAY_BANK);



  // Add an interrupt on the RESET_CV pin to catch rising edges
  attachInterrupt(RESET_CV, resetcv, RISING);
  
    osFlashFill();
}

// Called by interrupt on rising edge, for RESET_CV pin
void resetcv() {
  RESET_CHANGED = true;

  // Write osFlash array 

}

////////////////////////////////////////////////////
///////////////MAIN LOOP//////////////////////////
////////////////////////////////////////////////////

void loop() {
  //////////////////////////////////////////
  // IF FILE ENDS, RESTART FROM THE BEGINNING 
  //////////////////////////////////////////

  if (!playRaw1.isPlaying() && Looping){
    charFilename = buildPath(PLAY_BANK,PLAY_CHANNEL);
    playRaw1.playFrom(charFilename,0);   // change audio
    resetLedTimer = 0; // turn on Reset LED 

  }

  if (playRaw1.failed){
    Serial.println("ABOUT TO REBOOT");
    reBoot();
  }


  //////////////////////////////////////////
  ////////REACT TO ANY CHANGES /////////////
  //////////////////////////////////////////


  if (CHAN_CHANGED || RESET_CHANGED){
    if (MUTE){  
      fade1.fadeOut(DECLICK);      // fade out before change 
      delay(DECLICK);
    }

    charFilename = buildPath(PLAY_BANK,NEXT_CHANNEL);
    PLAY_CHANNEL = NEXT_CHANNEL;


    if (RESET_CHANGED == false && Looping) playhead = playRaw1.fileOffset(); // Carry on from previous position, unless reset pressed or time selected
    playhead = (playhead / 16) * 16; // scale playhead to 16 step chunks 
    playRaw1.playFrom(charFilename,playhead);   // change audio
//    delay(10);

    if (MUTE)    fade1.fadeIn(DECLICK);                          // fade back in   
    ledWrite(PLAY_BANK);
    CHAN_CHANGED = false;
    RESET_CHANGED = false; 
    resetLedTimer = 0; // turn on Reset LED 
  }



  //////////////////////////////////////////
  // CHECK INTERFACE  & UPDATE DISPLAYS/////  
  //////////////////////////////////////////

  osFlasher(); // FLASH OS NUMBER ON STARTUP 


  if (checkI >= checkFreq){
    checkInterface(); 
    checkI = 0;  
  }

  if (showDisplay > showFreq){
    //    playDisplay();
    //    whatsPlaying();
    Serial.print("failed=");
    Serial.println(playRaw1.failed);
    showDisplay = 0;
  }

  if (PLAY_BANK < 16){

    digitalWrite(RESET_LED, resetLedTimer < FLASHTIME); // flash reset LED 
  }
  else{
    digitalWrite(RESET_LED, resetLedTimer > FLASHTIME); // flash reset LED 

  }

  if (fps > 1000/peakFPS && meterDisplay > meterHIDE && ShowMeter) peakMeter();    // CALL PEAK METER   


}









