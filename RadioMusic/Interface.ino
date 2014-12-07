//////////////////////////////////////////
// CHECK INTERFACE POTS BUTTONS ETC //////
/////////////////////////////////////////

void checkInterface(){

  int channel; 
  int time; 
  int bank; 
  boolean channelChange = false; 
  boolean timeChange = false; 

  // READ POTS 

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

  // IDENTIFY CHANGES 

  int chanPotChange = abs(chanPot - chanPotOld); 
  int chanCVChange = abs(chanCV - chanCVOld);
  int timPotChange = abs(timPot - timPotOld);
  int timCVChange = abs(timCV - timCVOld); 

  if ( chanPotChange > chanHyst ){
    // Channel pot changed  
    channelChange = true; 
    CHAN_CHANGED = ChanPotImmediate;
    chanPotOld = chanPot; 

  }

  if (chanCVChange > chanHyst){
    // Channel CV Changed 
    channelChange = true; 
    CHAN_CHANGED = ChanCVImmediate;
    chanCVOld = chanCV; 

  }

  if (channelChange && chanChanged > chanHystTime){
    // Change the channel global variable 
    channel = chanPot + chanCV; 
    channel = constrain(channel, 0, 1023);
    channel = map(channel,0,1024,0,FILE_COUNT[PLAY_BANK]); // Highest pot value = 1 above what's possible (ie 1023+1) and file count is one above the number of the last file (zero indexed)  
    if (channel == PLAY_CHANNEL) CHAN_CHANGED = false; // Do not retrigger if channel is unchanged 
    PLAY_CHANNEL = channel;
    channelChange = false; 
    chanChanged = 0;
  }

  if (timPotChange > timHyst){
    // Time Pot Changed  
    timeChange = true;
    RESET_CHANGED = StartPotImmediate; 
   timPotOld = timPot; 
 
  }

  if (timCVChange > timHyst){
    // Time CV Changed  
    timeChange = true;
    RESET_CHANGED = StartCVImmediate;  
    timCVOld = timCV; 

  }

  if (timeChange && timChanged > timHystTime){
    // Do change the time global variable 
    time = timPot + timCV;   
    time = constrain(time, 0, 1023); 
    time = (time / StartCVDivider) * StartCVDivider; // Quantizes start position 
    unsigned long fileLength = FILE_SIZES[PLAY_BANK][PLAY_CHANNEL];
    unsigned long newTime = ((fileLength/1023) * time);
    unsigned long playPosition = playRaw1.fileOffset();
    unsigned long fileStart = (playPosition / fileLength) * fileLength;
    playhead = fileStart + newTime;
    timeChange = false; 
    timChanged = 0;
  }



  // Reset Button & CV 
  if ( resetSwitch.update() )  RESET_CHANGED = resetSwitch.read();
  if ( resetCv.update() ) RESET_CHANGED = resetCv.read();


  // Hold Reset Button to Change Bank 
  bankTimer = bankTimer * digitalRead(RESET_BUTTON);

  if (bankTimer > HOLDTIME){
    PLAY_BANK++;
    if (PLAY_BANK > ACTIVE_BANKS) PLAY_BANK = 0;
    if (PLAY_CHANNEL > FILE_COUNT[PLAY_BANK]) PLAY_CHANNEL = FILE_COUNT[PLAY_BANK];
    CHAN_CHANGED = true;
    bankTimer = 0;  
    meterDisplay = 0;
    EEPROM.write(BANK_SAVE, PLAY_BANK);
  }

}


