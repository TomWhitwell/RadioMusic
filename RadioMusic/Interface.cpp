#include "Interface.h"

#include "Arduino.h"
#include "Bounce2.h"

// SETUP VARS TO STORE CONTROLS
// A separate variable for tracking reset CV only
volatile boolean resetCVHigh = false;

// Called by interrupt on rising edge, for RESET_CV pin
void resetcv() {
	resetCVHigh = true;
}

void Interface::init(int fileSize, int channels, const Settings& settings, PlayState* state) {

    analogReadRes(ADC_BITS);
	pinMode(BANK_BUTTON, INPUT);
	pinMode(RESET_BUTTON, OUTPUT);
	pinMode(RESET_CV, settings.resetIsOutput ? OUTPUT : INPUT);

	// Add an interrupt on the RESET_CV pin to catch rising edges
	attachInterrupt(RESET_CV, resetcv, RISING);

	uint16_t bounceInterval = 5;
	resetButtonBounce.attach(RESET_BUTTON);
	resetButtonBounce.interval(bounceInterval);

	startCVDivider = settings.startCVDivider;

	chanPotImmediate = settings.chanPotImmediate;
	chanCVImmediate = settings.chanCVImmediate;

	startPotImmediate = settings.startPotImmediate;
	startCVImmediate = settings.startCVImmediate;

	currentFileSize = fileSize;
	channelCount = channels;

	playState = state;
	buttonTimer = 0;
	buttonHoldTime = 0;
	buttonHeld = false;
}

void Interface::setChannelCount(int count) {
	channelCount = count;
}

uint16_t Interface::update() {

	changes = 0;

	int channel;

	// READ & AVERAGE POTS

	int chanPot = 0;
	int chanCV = 0;
	int startPot = 0;
	int startCV = 0;

	for (int i = 0; i < SAMPLEAVERAGE; i++) {
		chanPot += analogRead(CHAN_POT_PIN);
		chanCV += analogRead(CHAN_CV_PIN);
		startPot += analogRead(TIME_POT_PIN);
		startCV += analogRead(TIME_CV_PIN);
	}

	chanPot = chanPot >> 4;
	chanCV = chanCV >> 4;
	startPot = startPot >> 4;
	startCV = startCV >> 4;

	// Snap small values to zero.
	if (startPot <= timeHysteresis >> 1)
		startPot = 0;
	if (startCV <= timeHysteresis >> 1)
		startCV = 0;

	// IDENTIFY POT / CV CHANGES

	boolean chanPotChange = (abs(chanPot - chanPotOld) > channelHysteresis);
	boolean chanCVChange = (abs(chanCV - chanCVOld) > channelHysteresis);
	boolean startPotChange = (abs(startPot - startPotOld) > timeHysteresis);
	boolean startCVChange = (abs(startCV - startCVOld) > timeHysteresis);

	// MAP INPUTS TO CURRENT SITUATION
	channel = chanPot + chanCV;
	channel = constrain(channel, 0, ADC_MAX_VALUE - 1);
	// Highest pot value = 1 above what's possible (ie ADC_MAX_VALUE+1)
	// and file count is one above the number of the last file (zero indexed)
	channel = map(channel, 0, ADC_MAX_VALUE, 0, channelCount);

	time = startPot + startCV;
	time = constrain(time, 0U, ADC_MAX_VALUE - 1);
	time = (time / startCVDivider) * startCVDivider; // Quantizes start position

	if (channel != playState->currentChannel) {
		playState->nextChannel = channel;
		changes |= CHANNEL_CHANGED;
		if((chanPotImmediate && chanPotChange) || (chanCVImmediate && chanCVChange)) {
			playState->channelChanged = true;
		}

		if(chanPotChange) {
			chanPotOld = chanPot;
		}
		if(chanCVChange) {
			chanCVOld = chanCV;
		}
	};

	if(startPotChange) {
		changes |= TIME_POT_CHANGED;
		if(startPotImmediate) {
			changes |= CHANGE_START_NOW;
		}
		startPotOld = startPot;
	}

	if(startCVChange) {
		changes |= TIME_CV_CHANGED;
		if(startCVImmediate) {
			changes |= CHANGE_START_NOW;
		}
		startCVOld = startCV;
	}

	changes |= updateButton();

	if(resetCVHigh || (changes & BUTTON_SHORT_PRESS)) {
		changes |= RESET_TRIGGERED;
	}
	resetCVHigh = false;

	return changes;
}

uint16_t Interface::updateButton() {

	resetButtonBounce.update();
	uint16_t buttonState = 0;

	// Button pressed
	if(resetButtonBounce.rose()) {
		buttonTimer = 0;
		buttonHeld = true;
	}

    if(resetButtonBounce.fell()) {
    	buttonHeld = false;
    	// button has been held down for some time
        if (buttonTimer >= SHORT_PRESS_DURATION && buttonTimer < LONG_PRESS_DURATION){
        	buttonState |= BUTTON_SHORT_PRESS;
        } else if(buttonTimer > LONG_PRESS_DURATION) {
        	buttonState |= BUTTON_LONG_RELEASE;
        }
        buttonTimer = 0;
    }

    if(buttonHeld && buttonTimer >= LONG_PRESS_DURATION) {
    	buttonState |= BUTTON_LONG_PRESS;

    	uint32_t diff = buttonTimer - LONG_PRESS_DURATION;
    	if(diff >= LONG_PRESS_PULSE_DELAY) {
    		buttonState |= BUTTON_PULSE;
    		buttonTimer = LONG_PRESS_DURATION;
    	}
    }

    return buttonState;
}
