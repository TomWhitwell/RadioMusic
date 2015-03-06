//////////////////////////////////////////
// SCAN SD DIRECTORIES INTO ARRAYS //////
/////////////////////////////////////////


int countDirectory(){ // returns number of number-labelled directories, assuming directories named 0,1,2,3,4,5 etc 
  AudioNoInterrupts();
  int d = 0;
    root = SD.open("/");
    root.rewindDirectory();
  while(true){
    File entry = root.openNextFile();
    if (! entry) break;
    String fileName = entry.name();
    if (entry.isDirectory()){
      if (fileName.toInt() != 0 || fileName == "0") d++;
    }
    entry.close();
  }
 AudioInterrupts();
 return d;
}


int loadFiles(int directory){ // returns number of files in the specified directory + loads FILE_NAMES[n] and FILE_SIZES[n]
AudioNoInterrupts();
   int f = 0;
  String liveFilename = "/";
  liveFilename += directory;
  liveFilename += "/";
  char name[5];
  liveFilename.toCharArray(name,4);
  if (!SD.exists(name)) {
    Serial.println("directory not found") ;
    return 0;
 
  };
  
  root = SD.open(name);
  root.rewindDirectory();
  
  while(true){
    File entry = root.openNextFile();
    if (! entry) break;
    String fileName = entry.name();
    if (fileName.endsWith(FILE_TYPE) && !fileName.startsWith("_")){
      FILE_NAMES[f] = entry.name();
      FILE_SIZES[f] = entry.size();
      f++;
    }
    entry.close();
  }
 AudioInterrupts();
  return f;
}





// TAKE BANK AND CHANNEL AND RETURN PROPERLY FORMATTED PATH TO THE FILE 
// = /1/FILENAME.RAW 
char* buildPath (int bank, int channel){

  // String liveFilename = bank;  
//  liveFilename += "/";
  
   
  String liveFilename = "/";
  liveFilename += bank;  
  liveFilename += "/";
  
  
  liveFilename += FILE_NAMES[channel];
  char filename[19];
  liveFilename.toCharArray(filename, 18);
//  Serial.print("Built filename: ");
//  Serial.println(filename);
  return filename;
 
}

void reBoot(){
  delay (500);
  WRITE_RESTART(0x5FA0004);
}


//////////////////////////////////////////////
////////VARIOUS DEBUG DISPLAY FUNCTIONS //////
//////////////////////////////////////////////

// SHOW VISUAL INDICATOR OF TRACK PROGRESS IN SERIAL MONITOR 
void playDisplay(){
  int position = (playRaw1.fileOffset() %  FILE_SIZES[PLAY_CHANNEL]) >> 21;
  int size = FILE_SIZES[PLAY_CHANNEL] >> 21;
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




void printSettings(){
  Serial.print("MUTE=");
  Serial.println(MUTE);
  Serial.print("DECLICK=");
  Serial.println(DECLICK);
  Serial.print("ShowMeter=");
  Serial.println(ShowMeter);
  Serial.print("meterHIDE=");
  Serial.println(meterHIDE);
  Serial.print("ChanPotImmediate=");
  Serial.println(ChanPotImmediate);
  Serial.print("ChanCVImmediate=");
  Serial.println(ChanCVImmediate);
  Serial.print("StartPotImmediate=");
  Serial.println(StartPotImmediate);
  Serial.print("StartCVImmediate=");
  Serial.println(StartCVImmediate);
}









// DISPLAY PEAK METER IN LEDS 
void peakMeter(){
  if (peak1.available()) {
    fps = 0;
    float peakReading = peak1.read();
    int monoPeak = round ( peakReading * 4);
    monoPeak = round (pow(2,monoPeak));
    ledWrite(monoPeak-1); // 
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
////READ AND WRITE SETTINGS ON THE SD CARD ///
////VIA http://overskill.alexshu.com/?p=107 ///
//////////////////////////////////////////////




void readSDSettings(){
  char character;
  String settingName;
  String settingValue;
  settingsFile = SD.open("settings.txt");
  if (settingsFile) {
    while (settingsFile.available()) {
      character = settingsFile.read();
      while(character != '='){
        settingName = settingName + character;
        character = settingsFile.read();
      }
      character = settingsFile.read();
      while(character != '\n'){
        settingValue = settingValue + character;
        character = settingsFile.read();
        if(character == '\n'){
          /*
          //Debuuging Printing
           Serial.print("Name:");
           Serial.println(settingName);
           Serial.print("Value :");
           Serial.println(settingValue);
           */
          // Apply the value to the parameter
          applySetting(settingName,settingValue);
          // Reset Strings
          settingName = "";
          settingValue = "";
        }
      }
    }
    // close the file:
    settingsFile.close();
  } 
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening settings.txt");
  }
}
/* Apply the value to the parameter by searching for the parameter name
 Using String.toInt(); for Integers
 toFloat(string); for Float
 toBoolean(string); for Boolean
 */
void applySetting(String settingName, String settingValue) {

  if(settingName == "MUTE") {
    MUTE=toBoolean(settingValue);
  }

  if(settingName == "DECLICK") {
    DECLICK=settingValue.toInt();
  }

  if(settingName == "ShowMeter") {
    ShowMeter=toBoolean(settingValue);
  }

  if(settingName == "meterHIDE") {
    meterHIDE=settingValue.toInt();
  }

  if(settingName == "ChanPotImmediate") {
    ChanPotImmediate=toBoolean(settingValue);
  }

  if(settingName == "ChanCVImmediate") {
    ChanCVImmediate=toBoolean(settingValue);
  }

  if(settingName == "StartPotImmediate") {
    StartPotImmediate=toBoolean(settingValue);
  }

  if(settingName == "StartCVImmediate") {
    StartCVImmediate=toBoolean(settingValue);
  }

  if(settingName == "StartCVDivider") {
    StartCVDivider=settingValue.toInt();;
  }

  if(settingName == "Looping") {
    Looping=toBoolean(settingValue);
  }


}
// converting string to Float
float toFloat(String settingValue){
  char floatbuf[settingValue.length()];
  settingValue.toCharArray(floatbuf, sizeof(floatbuf));
  float f = atof(floatbuf);
  return f;
}
// Converting String to integer and then to boolean
// 1 = true
// 0 = false
boolean toBoolean(String settingValue) {
  if(settingValue.toInt()==1){
    return true;
  } 
  else {
    return false;
  }
}
// Writes A Configuration file
void writeSDSettings() {
  // Delete the old One
  SD.remove("settings.txt");
  // Create new one
  settingsFile = SD.open("settings.txt", FILE_WRITE);
  // writing in the file works just like regular print()/println() function
  settingsFile.print("MUTE=");
  settingsFile.println(MUTE);
  settingsFile.print("DECLICK=");
  settingsFile.println(DECLICK);
  settingsFile.print("ShowMeter=");
  settingsFile.println(ShowMeter);
  settingsFile.print("meterHIDE=");
  settingsFile.println(meterHIDE);
  settingsFile.print("ChanPotImmediate=");
  settingsFile.println(ChanPotImmediate);
  settingsFile.print("ChanCVImmediate=");
  settingsFile.println(ChanCVImmediate);
  settingsFile.print("StartPotImmediate=");
  settingsFile.println(StartPotImmediate);
  settingsFile.print("StartCVImmediate=");
  settingsFile.println(StartCVImmediate);
  settingsFile.print("StartCVDivider=");
  settingsFile.println(StartCVDivider);
  settingsFile.print("Looping=");
  settingsFile.println(Looping);




  // close the file:
  settingsFile.close();
  //Serial.println("Writing done.");
}












