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
    if (TunePotImmediate) CHAN_CHANGED = true;
    CHAN_CHANGED_TIME = millis();
  }
  // Time pot & CV 
  int averages = 5; // how many readings to take, to get average
  int timePot = 0;
  for(int a = 0; a < averages; a++){
    timePot += analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN);
  }
  timePot = timePot / averages; 
  timePot = (timePot / StartCVDivider)*StartCVDivider; // Quantizes start position 
  timePot = constrain(timePot, 0, 1023); 
  elapsed = millis() - CHAN_CHANGED_TIME;

  if (abs(timePot - timePotOld) > TIME_HYSTERESIS && elapsed > HYSTERESIS){
    unsigned long fileLength = FILE_SIZES[PLAY_BANK][PLAY_CHANNEL];
    unsigned long newTime = ((fileLength/1023) * timePot);
    unsigned long playPosition = playRaw1.fileOffset();
    unsigned long fileStart = (playPosition / fileLength) * fileLength;
    playhead = fileStart + newTime;
    if (StartPotImmediate) RESET_CHANGED = true;  
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

