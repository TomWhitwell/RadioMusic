#ifndef Interface_h
#define Interface_h

#include "Arduino.h"

#include <Bounce2.h>
#include "PlayState.h"
#include "Settings.h"

#define CHAN_POT_PIN A9 	// pin for Channel pot
#define CHAN_CV_PIN A6 		// pin for Channel CV
#define TIME_POT_PIN A7 	// pin for Time pot
#define TIME_CV_PIN A8 		// pin for Time CV
#define RESET_BUTTON 8 		// Reset button
#define RESET_CV 9 		// Reset pulse input
#define BANK_BUTTON 2 // Bank Button

#define TIME_POT_CHANGED 1
#define TIME_CV_CHANGED 2
#define CHANNEL_CHANGED 4
#define CHANGE_START_NOW 8
#define BUTTON_SHORT_PRESS 16
#define BUTTON_LONG_PRESS 32
#define BUTTON_LONG_RELEASE 64
#define BUTTON_PULSE 128
#define RESET_TRIGGERED 256

#define SHORT_PRESS_DURATION 10
#define LONG_PRESS_DURATION 1000
// after LONG_PRESS_DURATION every LONG_PRESS_PULSE_DELAY milliseconds the update
// function will set BUTTON_PULSE
#define LONG_PRESS_PULSE_DELAY 600

#define SAMPLEAVERAGE   16 	// How many values are read and averaged of pot/CV inputs each interface check.

#define ADC_BITS 13
#define ADC_MAX_VALUE (1 << ADC_BITS)

class Interface {
public:

	int channelHysteresis = 32; // how many steps to move before making a change (out of 1024 steps on a reading)
	int timeHysteresis = 32;

	int chanPotOld = 0;
	int chanCVOld = 0;
	int startPotOld = 0;
	int startCVOld = 0;

	boolean buttonHeld = false;
	unsigned long time = 0;

	elapsedMillis buttonHoldTime;

	PlayState* playState;

	Interface() {
		playState = NULL;
	}

	void init(int fileSize, int channels, const Settings& settings, PlayState* state);
	void setChannelCount(int count);
	uint16_t update();
	uint16_t updateButton();
private:

	Bounce resetButtonBounce;
	elapsedMillis buttonTimer = 0;

	uint16_t changes = 0;

	uint16_t startCVDivider = 1;
	boolean chanPotImmediate = true;
	boolean chanCVImmediate = true;
	boolean startPotImmediate = true;
	boolean startCVImmediate = true;

	int currentFileSize = 0;
	int channelCount = 0;
};

#endif
