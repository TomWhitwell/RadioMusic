/*
RADIO MUSIC 
 
 Audio out: Onboard DAC, teensy3.1 pin A14/DAC
 
 Bank Button: 2
 Bank LEDs 3,4,5,6
 Reset Button: 8  
 Reset LED 11 
 Reset CV input: 9 
 
 Channel Pot: A9 
 Channel CV: A8 
 Time Pot: A7 
 Time CV: A6 
 
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
AudioPlaySdRaw           playRaw1;       //xy=131,81
AudioEffectFade          fade1;          //xy=257,169
AudioOutputAnalog        dac1;           //xy=334,98
AudioConnection          patchCord1(playRaw1, fade1);
AudioConnection          patchCord2(fade1, dac1);
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
#define BLOCK_SIZE 2 // size of blocks to read - must be more than 1, performance might improve with 16?

// SETUP VARS TO STORE CONTROLS 
#define CHAN_POT_PIN 9 // pin for Channel pot
#define CHAN_CV_PIN 8 // pin for Channel CV 
#define TIME_POT_PIN 7 // pin for Time pot
#define TIME_CV_PIN 6 // pin for Time CV
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
unsigned long loopcount; 
unsigned long playhead;
char* charFilename;


// BANK SWITCHER SETUP 
#define BANK_BUTTON 2 // Bank Button 
#define LED0 3
#define LED1 4
#define LED2 5
#define LED3 6
Bounce bankSwitch = Bounce( BANK_BUTTON, 20 ); 
int PLAY_BANK = 0; 


// CHANGE HOW INTERFACE REACTS 
#define HYSTERESIS 10 // MINIMUM MILLIS BETWEEN CHANGES
#define TIME_HYSTERESIS 4 // MINIMUM KNOB POSITIONS MOVED 
#define DECLICK 10 // milliseconds of fade in/out on switching 
#define FLASHTIME 10 // How long do LEDs flash for? 
#define HOLDTIME 400 // How many millis to hold a button to get 2ndary function? 
elapsedMillis showDisplay; // elapsedMillis is a special variable in Teensy - increments every millisecond 
elapsedMillis resetLedTimer = 0;
elapsedMillis bankTimer = 0;

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



////////////////////////////////////////////////////
// /////////////MAIN LOOP//////////////////////////
////////////////////////////////////////////////////

void loop() {

  // CHECK INTERFACE EVERY x LOOPS OF THE MAIN ROUTINE   
  if (loopcount>200){
    checkInterface(); 
    loopcount = 0;  

  }

  // UPDATE SERIAL PORT DISPLAY EVERY x MILLISECONDS   
  if (showDisplay > 250){
    playDisplay();
    showDisplay = 0;
  }

  digitalWrite(RESET_LED, resetLedTimer < FLASHTIME);


  // IF ANYTHING CHANGES, DO THIS
  if (CHAN_CHANGED || RESET_CHANGED){
   
    fade1.fadeOut(DECLICK);      // fade out before change 
    delay(DECLICK);
   
    charFilename = buildPath(PLAY_BANK,PLAY_CHANNEL);
    if (RESET_CHANGED == false) playhead = playRaw1.fileOffset(); // Carry on from previous position, unless reset pressed
//    if (playhead %2) playhead--; // odd playhead starts = white noise 
playhead = (playhead / 16) * 16; 
    playRaw1.playFrom(charFilename,playhead);   // change audio 
    fade1.fadeIn(DECLICK);                          // fade back in 

    ledWrite(pow(2,PLAY_BANK));
    CHAN_CHANGED = false;
    RESET_CHANGED = false; 
    resetLedTimer = 0; // turn on Reset LED 

  }



  // IF FILE ENDS, RESTART FROM THE BEGINNING 
  CHAN_CHANGED = !playRaw1.isPlaying();

  loopcount++;
}


////////////////////////////////////////////////////
// /////////////FUNCTIONS//////////////////////////
////////////////////////////////////////////////////




// WRITE A 4 DIGIT BINARY NUMBER TO LED0-LED3 
void ledWrite(int n){
  digitalWrite(LED0, HIGH && (n & B00001000));
  digitalWrite(LED1, HIGH && (n & B00000100));
  digitalWrite(LED2, HIGH && (n & B00000010));
  digitalWrite(LED3, HIGH && (n & B00000001)); 
}



// READ AND SCALE POTS AND CVs AND RETURN TO GLOBAL VARIBLES 

void checkInterface(){
  unsigned long elapsed;
  // Channel Pot 
  int channel = analogRead(CHAN_POT_PIN) + analogRead(CHAN_CV_PIN);
  channel = constrain(channel, 0, 1024);
  channel = map(channel,0,1024,0,FILE_COUNT[PLAY_BANK]);
  elapsed = millis() - CHAN_CHANGED_TIME;
  if (channel != PLAY_CHANNEL && elapsed > HYSTERESIS) {
    PLAY_CHANNEL = channel;
    CHAN_CHANGED = true;
    CHAN_CHANGED_TIME = millis();
  }

  // Time pot 
  int timePot = analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN);
  timePot = constrain(timePot, 0, 1024); 
  elapsed = millis() - CHAN_CHANGED_TIME;

  if (abs(timePot - timePotOld) > TIME_HYSTERESIS && elapsed > HYSTERESIS){


    unsigned long fileLength = FILE_SIZES[PLAY_BANK][PLAY_CHANNEL];
    unsigned long newTime = ((fileLength/1024) * timePot);
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

  // Hold Reset to Change Bank 
  bankTimer = bankTimer * digitalRead(RESET_BUTTON);
  if (bankTimer > HOLDTIME){
    PLAY_BANK++;
    if (PLAY_BANK >= BANKS) PLAY_BANK = 0;   
    CHAN_CHANGED = true;
    bankTimer = 0;  
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


// Replacement Map function to deal with very large numbers 
unsigned long myMap(unsigned long x, unsigned long in_min, unsigned long in_max, unsigned long out_min, unsigned long out_max) {

  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void whatsPlaying (){
  Serial.print("Bank:");
  Serial.print(PLAY_BANK);
  Serial.print(" Channel:");
  Serial.print(PLAY_CHANNEL);  
  Serial.print(" File:");  
  Serial.println (charFilename); 
}

void playDisplay(){
  int position = (playRaw1.fileOffset() %  FILE_SIZES[PLAY_BANK][PLAY_CHANNEL]) >> 21;
  int size = FILE_SIZES[PLAY_BANK][PLAY_CHANNEL] >> 21;


  for (int i = 0; i < size; i++){
    if (i == position) Serial.print("|");
    else Serial.print("_");
  }

  //  Serial.println(charFilename);




}

