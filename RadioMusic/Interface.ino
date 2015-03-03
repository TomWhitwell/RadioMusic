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

Serial.print("changes:");
Serial.print(chanPotChange);
Serial.print(chanCVChange);
Serial.print(timPotChange);
Serial.println(timCVChange);

  // MAP INPUTS TO CURRENT SITUATION 

  channel = chanPot + chanCV; 
  channel = constrain(channel, 0, 1023);
  channel = map(channel,0,1024,0,FILE_COUNT[PLAY_BANK]); // Highest pot value = 1 above what's possible (ie 1023+1) and file count is one above the number of the last file (zero indexed)  

  time = timPot + timCV;   
  time = constrain(time, 0, 1023); 
  time = (time / StartCVDivider) * StartCVDivider; // Quantizes start position 
  time  = time * (FILE_SIZES[PLAY_BANK][PLAY_CHANNEL]/1023);


  // IDENTIFY AND DEPLOY RELEVANT CHANGES  

  if (channel != PLAY_CHANNEL && chanPotChange) {
    NEXT_CHANNEL = channel; 
    CHAN_CHANGED = StartPotImmediate;
    chanPotOld = chanPot;
  };

  if (channel != PLAY_CHANNEL && chanCVChange) {
    NEXT_CHANNEL = channel; 
    CHAN_CHANGED = StartCVImmediate;
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
    if (PLAY_BANK > ACTIVE_BANKS) PLAY_BANK = 0;
    if (PLAY_CHANNEL >= FILE_COUNT[PLAY_BANK]) PLAY_CHANNEL = FILE_COUNT[PLAY_BANK]-1;
    CHAN_CHANGED = true;
    bankTimer = 0;  
    meterDisplay = 0;
    EEPROM.write(BANK_SAVE, PLAY_BANK);
  }

}





