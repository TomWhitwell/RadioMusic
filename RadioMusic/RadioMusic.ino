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

 Additions and changes:
 2016 by Jouni Stenroos - jouni.stenroos@iki.fi 
 - New bank change mode
 - Removing 330 file limit
 - Improving reset
 - File sorting
 - Audio crossfade
 - Some refactoring and organization of code.
 
 */
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RadioMusic.h"
#include "AudioSystemHelpers.h"
#include "Settings.h"
#include "LedControl.h"
#include "FileScanner.h"
#include "AudioEngine.h"
#include "Interface.h"
#include "PlayState.h"

#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif

// Press reset button to reboot
//#define RESET_TO_REBOOT
//#define ENGINE_TEST

#define EEPROM_BANK_SAVE_ADDRESS 0

#define FLASHTIME 	10  	// How long do LEDs flash for?
#define SHOWFREQ 	250 	// how many millis between serial Debug updates

#define peakFPS 30   //  FRAMERATE FOR PEAK METER

#define SD_CARD_CHECK_DELAY 20

// //////////
// TIMERS
// //////////

elapsedMillis showDisplay;
elapsedMillis resetLedTimer = 0;
elapsedMillis ledFlashTimer = 0;

elapsedMillis meterDisplayDelayTimer; // Counter to hide MeterDisplay after bank change
elapsedMillis peakDisplayTimer; // COUNTER FOR PEAK METER FRAMERATE


int prevBankTimer = 0;
boolean flashLeds = false;
boolean bankChangeMode = false;
File settingsFile;

Settings settings("SETTINGS.TXT");
LedControl ledControl;
FileScanner fileScanner;
AudioEngine audioEngine;
Interface interface;
PlayState playState;

int NO_FILES = 0;
uint8_t noFilesLedIndex = 0;

uint8_t rebootCounter = 0;

void setup() {

#ifdef DEBUG_STARTUP
	while( !Serial );
	Serial.println("Starting");
#endif // DEBUG_STARTUP

	ledControl.init();
	ledControl.single(playState.bank);

	// SD CARD SETTINGS FOR AUDIO SHIELD
	SPI.setMOSI(7);
	SPI.setSCK(14);

	boolean hasSD = openSDCard();
	if(!hasSD) {
		Serial.println("Rebooting");
		reBoot(0);
	}

	settings.init(hasSD);

	File root = SD.open("/");
	fileScanner.scan(&root, settings);

	getSavedBankPosition();

	audioEngine.init(settings);

	int numFiles = 0;
	for(int i=0;i<=fileScanner.lastBankIndex;i++) {
		numFiles += fileScanner.numFilesInBank[i];
	}
	D(Serial.print("File Count ");Serial.println(numFiles););

	if(numFiles == 0) {
		NO_FILES = 1;
		D(Serial.println("No files"););
		ledFlashTimer = 0;
	} else if(fileScanner.numFilesInBank[playState.bank] == 0) {
		D(Serial.println("Empty bank"););
		while(fileScanner.numFilesInBank[playState.bank] == 0) {
			playState.bank++;
			if(playState.bank == fileScanner.lastBankIndex) {
				playState.bank = 0;
			}
		}
		D(Serial.print("Set bank to ");Serial.println(playState.bank););
	}

	interface.init(fileScanner.fileInfos[playState.bank][0].size, fileScanner.numFilesInBank[playState.bank], settings, &playState);

	D(Serial.println("--READY--"););
}

void getSavedBankPosition() {
	// CHECK  FOR SAVED BANK POSITION
	int a = 0;
	a = EEPROM.read(EEPROM_BANK_SAVE_ADDRESS);
	if (a >= 0 && a <= fileScanner.lastBankIndex) {
		D(
			Serial.print("Using bank from EEPROM ");
			Serial.print(a);
			Serial.print(" . Active banks ");
			Serial.println(fileScanner.lastBankIndex);

		);
		playState.bank = a;
		playState.channelChanged = true;
	} else {
		EEPROM.write(EEPROM_BANK_SAVE_ADDRESS, 0);
	};
}

boolean openSDCard() {
	if (!(SD.begin(SS))) {

		Serial.println("No SD.");
		while (!(SD.begin(SS))) {
			ledControl.single(15);
			delay(SD_CARD_CHECK_DELAY);
			ledControl.single(rebootCounter % 4);
			delay(SD_CARD_CHECK_DELAY);
			rebootCounter++;
			Serial.print("Crash Countdown ");
			Serial.println(rebootCounter);
			if (rebootCounter > 4) {
				return false;
			}
		}
	}
	return true;
}

void loop() {

	#ifdef CHECK_CPU
	checkCPU();
//	audioEngine.measure();
	#endif

	if(NO_FILES) {
		// TODO : Flash the lights to show there are no files
		if(ledFlashTimer > 100) {
			ledControl.single(noFilesLedIndex);
			noFilesLedIndex++;
			noFilesLedIndex %= 4;
			ledFlashTimer = 0;
			rebootCounter ++;
			// Wait 3 seconds and reboot
			if(rebootCounter == 30) {
				reBoot(0);
			}
		}
		return;
	}

	updateInterfaceAndDisplay();

	audioEngine.update();

	if(audioEngine.error) {
		// Too many read errors, reboot
		Serial.println("Audio Engine errors. Reboot");
		reBoot(0);
	}

	if (playState.channelChanged) {
		D(
		Serial.print("RM: Going to next channel : ");
		if(playState.channelChanged) Serial.print("RM: Channel Changed. ");
		Serial.println("");
		);

		playState.currentChannel = playState.nextChannel;

		AudioFileInfo* currentFileInfo = &fileScanner.fileInfos[playState.bank][playState.nextChannel];

		audioEngine.changeTo(currentFileInfo, interface.start);
		playState.channelChanged = false;

		resetLedTimer = 0;

	}

}

void updateInterfaceAndDisplay() {

	uint16_t changes = checkInterface();
	updateDisplay(changes);
}

void updateDisplay(uint16_t changes) {
	if (showDisplay > SHOWFREQ) {
		showDisplay = 0;
	}
	if (bankChangeMode) {
		ledControl.showReset(1);// Reset led is on continuously when in bank change mode..
		if(!flashLeds) {
			ledControl.multi(playState.bank);
		}

	} else {
		ledControl.showReset(resetLedTimer < FLASHTIME); // flash reset LED
	}

	if (flashLeds) {
		if (ledFlashTimer < FLASHTIME * 4) {
			ledControl.multi(0x0F);
		} else if(ledFlashTimer < FLASHTIME * 8) {
			ledControl.multi(0);
		} else {
			ledFlashTimer = 0;
		}
	} else if (settings.showMeter && !bankChangeMode) {
		peakMeter();
	}
}

// INTERFACE //

uint16_t checkInterface() {

	uint16_t changes = interface.update();

	#ifdef RESET_TO_REBOOT
	if (changes & BUTTON_SHORT_PRESS) {
		reBoot(0);
	}
	#endif

	// BANK MODE HANDLING
	if((changes & BUTTON_LONG_PRESS) && !bankChangeMode) {
		D(Serial.println("Enter bank change mode"););
		bankChangeMode = true;
		nextBank();
//		ledFlashTimer = 0;
	} else if((changes & BUTTON_LONG_RELEASE) && bankChangeMode) {
		D(Serial.println("Exit bank change mode"););
		flashLeds = false;
		bankChangeMode = false;
	}

	if(changes & BUTTON_PULSE) {
//		flashLeds = false;
		if(bankChangeMode) {
			D(Serial.println("BUTTON PULSE"););
			nextBank();
		} else {
			D(Serial.println("Button Pulse but not in bank mode"););
		}

	}

	boolean resetTriggered = changes & RESET_TRIGGERED;

	bool skipToStartPoint = false;
	bool speedChange = false;

	switch(settings.pitchMode) {
		default:
		case Settings::PitchMode::Start: {
			if((changes & CHANGE_START_NOW) || resetTriggered) {
				skipToStartPoint = true;
			}
			break;
		}
		case Settings::PitchMode::Speed: {
			if(resetTriggered) {
				skipToStartPoint = true;
			}

			if((changes & (ROOT_NOTE_CHANGED | ROOT_POT_CHANGED | ROOT_CV_CHANGED) ) || resetTriggered) {
				speedChange = true;
			}
			break;
		}
		case Settings::PitchMode::PotSpeedCvStart: {
			if((changes & CHANGE_START_NOW) || resetTriggered) {
				skipToStartPoint = true;
			}

			if((changes & (ROOT_NOTE_CHANGED | ROOT_POT_CHANGED) ) || resetTriggered) {
				speedChange = true;
			}
			break;
		}
	}

	if(resetTriggered) {
		if((changes & CHANNEL_CHANGED) || playState.nextChannel != playState.currentChannel) {
			playState.channelChanged = true;
		} else {
			resetLedTimer = 0;
		}
	}

	if(speedChange) doSpeedChange();
	if(skipToStartPoint && !playState.channelChanged) {
		if(settings.pitchMode == Settings::PitchMode::Speed) {
			audioEngine.skipTo(0);
		} else {
			D(Serial.print("Skip to ");Serial.println(interface.start););
			audioEngine.skipTo(interface.start);
		}
	}

	return changes;
}

void doSpeedChange() {
	float speed = 1.0;
	speed = interface.rootNote - settings.rootNote;
	D(Serial.print("Root ");Serial.println(interface.rootNote););
	speed = pow(2,speed / 12);

	audioEngine.setPlaybackSpeed(speed);
}

void nextBank() {

	if(fileScanner.lastBankIndex == 0) {
		D(Serial.println("Only 1 bank."););
		return;
	}
	playState.bank++;
	if (playState.bank > fileScanner.lastBankIndex) {
		playState.bank = 0;
	}
	if(fileScanner.numFilesInBank[playState.bank] == 0) {
		D(Serial.print("No file in bank ");Serial.println(playState.bank););
		nextBank();
	}

	if (playState.nextChannel >= fileScanner.numFilesInBank[playState.bank])
		playState.nextChannel = fileScanner.numFilesInBank[playState.bank] - 1;
	interface.setChannelCount(fileScanner.numFilesInBank[playState.bank]);
	playState.channelChanged = true;

	D(
		Serial.print("RM: Next Bank ");
		Serial.println(playState.bank);
	);

	meterDisplayDelayTimer = 0;
	EEPROM.write(EEPROM_BANK_SAVE_ADDRESS, playState.bank);
}

#ifdef ENGINE_TEST
boolean tested = false;
int testIndex = 0;

void engineTest() {

	if(!tested) {
		audioEngine.test(fileScanner.fileInfos[playState.bank][0],fileScanner.fileInfos[playState.bank][1]);
		tested = true;
	}

	uint8_t changes = interface.update();

	if(changes & BUTTON_SHORT_PRESS) {
		testIndex += 2;
		if(testIndex >= fileScanner.numFilesInBank[playState.bank]) {
			Serial.println("Back to start");
			testIndex = 0;
		}
		audioEngine.test(fileScanner.fileInfos[playState.bank][testIndex],fileScanner.fileInfos[playState.bank][testIndex+1]);
	}

	return;
}
#endif

void peakMeter() {
	if( (peakDisplayTimer < 50) || (meterDisplayDelayTimer < settings.meterHide) ) return;

	float peakReading = audioEngine.getPeak();
	int monoPeak = round(peakReading * 4);
	monoPeak = round(pow(2, monoPeak));
//	D(Serial.print("Peak ");Serial.print(peakReading,4);Serial.print(" ");Serial.println(monoPeak););
	ledControl.multi(monoPeak - 1);
	peakDisplayTimer = 0;
}

