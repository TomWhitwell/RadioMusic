//////////////////////////////////////////
// CHECK INTERFACE POTS BUTTONS ETC //////
/////////////////////////////////////////

void checkInterface(){

  int channel; 
  unsigned long time; 

  // READ & AVERAGE POTS 

  int chanPot = 0; 
  int chanCV = 0; 
  int timPot = 0; 
  int timCV = 0; 

  for (int i = 0; i < sampleAverage; i++){
    chanPot += analogRead(CHAN_POT_PIN); 
    chanCV += analogRead(CHAN_CV_PIN); 
    timPot += analogRead(TIME_POT_PIN); 
    timCV += analogRead(TIME_CV_PIN); 
  }

  chanPot = chanPot / sampleAverage; 
  chanCV = chanCV / sampleAverage; 
  timPot = timPot / sampleAverage; 
  timCV = timCV / sampleAverage; 


  // IDENTIFY POT / CV CHANGES 

  boolean chanPotChange = (abs(chanPot - chanPotOld) > chanHyst); 
  boolean chanCVChange = (abs(chanCV - chanCVOld) > chanHyst);
  boolean timPotChange = (abs(timPot - timPotOld) > timHyst);
  boolean timCVChange = (abs(timCV - timCVOld) > timHyst); 


  // MAP INPUTS TO CURRENT SITUATION 

  channel = chanPot + chanCV; 
  channel = constrain(channel, 0, 1023);
  channel = map(channel,0,1024,0,FILE_COUNT); // Highest pot value = 1 above what's possible (ie 1023+1) and file count is one above the number of the last file (zero indexed)  

  time = timPot + timCV;   
  time = constrain(time, 0, 1023); 
  time = (time / StartCVDivider) * StartCVDivider; // Quantizes start position 
  time  = time * (FILE_SIZES[PLAY_CHANNEL] / 1023);


  // IDENTIFY AND DEPLOY RELEVANT CHANGES  

  if (channel != PLAY_CHANNEL && chanPotChange) {
    NEXT_CHANNEL = channel; 
    CHAN_CHANGED = ChanPotImmediate;
    chanPotOld = chanPot;
  };

  if (channel != PLAY_CHANNEL && chanCVChange) {
    NEXT_CHANNEL = channel; 
    CHAN_CHANGED = ChanCVImmediate;
    chanCVOld = chanCV;
  };

  if (timPotChange){
    playhead = time;
    RESET_CHANGED =  StartPotImmediate;
    timPotOld = timPot;
  }

  if (timCVChange){
    playhead = time;
    RESET_CHANGED =  StartCVImmediate;
    timCVOld = timCV;
  }









  // Reset Button & CV 
  if ( resetSwitch.update() )  RESET_CHANGED = resetSwitch.read();

  // Hold Reset Button to Change Bank 
  bankTimer = bankTimer * digitalRead(RESET_BUTTON);

  if (bankTimer > HOLDTIME){
    PLAY_BANK++;
    if (PLAY_BANK > ACTIVE_BANKS-1) PLAY_BANK = 0;
    
    Serial.print("Switching to bank ");
   Serial.println(PLAY_BANK); 


    FILE_COUNT = loadFiles(PLAY_BANK);
if (FILE_COUNT < 1) {
Serial.println("NO FILES ERROR");
}  
    
    Serial.print (FILE_COUNT);
Serial.print(" files found in bank ");
Serial.println (PLAY_BANK);
    Serial.print("attempting to play file ");
    Serial.println(NEXT_CHANNEL);
    
    if (NEXT_CHANNEL > FILE_COUNT-1) NEXT_CHANNEL = FILE_COUNT-1;
    if (NEXT_CHANNEL < 0) NEXT_CHANNEL = 0; 
    Serial.print("or maybe ");
    Serial.println(NEXT_CHANNEL);
    
    
    CHAN_CHANGED = true;
    bankTimer = 0;  
    meterDisplay = 0;
    EEPROM.write(BANK_SAVE, PLAY_BANK);
  }

}





