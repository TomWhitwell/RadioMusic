#include "Interface.h"

#include "Arduino.h"
#include "Bounce2.h"
#include "RadioMusic.h"

#ifdef DEBUG_INTERFACE
#define D(x) x
#else
#define D(x)
#endif

// SETUP VARS TO STORE CONTROLS
// A separate variable for tracking reset CV only
volatile boolean resetCVHigh = false;

// Called by interrupt on rising edge, for RESET_CV pin
void resetcv() {
	resetCVHigh = true;
}

void Interface::init(int fileSize, int channels, const Settings& settings, PlayState* state) {

    analogReadRes(ADC_BITS);
	pinMode(RESET_BUTTON, OUTPUT);
	pinMode(RESET_CV, settings.resetIsOutput ? OUTPUT : INPUT);

	// Add an interrupt on the RESET_CV pin to catch rising edges
	attachInterrupt(RESET_CV, resetcv, RISING);

	uint16_t bounceInterval = 5;
	resetButtonBounce.attach(RESET_BUTTON);
	resetButtonBounce.interval(bounceInterval);

	startCVDivider = settings.startCVDivider;

	pitchMode = settings.pitchMode;

    if(pitchMode) {
        quantiseRootCV = settings.quantiseRootCV;
        quantiseRootPot = settings.quantiseRootPot;

        float lowNote = settings.lowNote + 0.5;
        startCVInput.setRange(lowNote, lowNote + settings.noteRange, quantiseRootCV);
        startPotInput.setRange(0.0,48, quantiseRootPot);
        startCVInput.borderThreshold = 32;
        startPotInput.borderThreshold = 32;
    } else {
    	D(Serial.print("Set Start Range ");Serial.println(ADC_MAX_VALUE / startCVDivider););
    	startPotInput.setRange(0.0, ADC_MAX_VALUE / startCVDivider, false);
    	startCVInput.setRange(0.0, ADC_MAX_VALUE / startCVDivider, false);
        startPotInput.setAverage(true);
        startCVInput.setAverage(true);
    }

	channelPotImmediate = settings.chanPotImmediate;
	channelCVImmediate = settings.chanCVImmediate;

	startPotImmediate = settings.startPotImmediate;
	startCVImmediate = settings.startCVImmediate;

	setChannelCount(channels);

	playState = state;
	buttonTimer = 0;
	buttonHoldTime = 0;
	buttonHeld = false;
}

void Interface::setChannelCount(uint16_t count) {
	channelCount = count;
	channelCVInput.setRange(0, channelCount - 1, true);
	channelPotInput.setRange(0, channelCount - 1, true);
	D(Serial.print("Channel Count ");Serial.println(channelCount););
}

uint16_t Interface::update() {

	uint16_t channelChanged = updateChannelControls();
	uint16_t startChanged = pitchMode ? updateRootControls() : updateStartControls();

	changes = channelChanged;
	changes |= startChanged;
	changes |= updateButton();

	if(resetCVHigh || (changes & BUTTON_SHORT_PRESS)) {
		changes |= RESET_TRIGGERED;
	}
	resetCVHigh = false;

	return changes;
}

uint16_t Interface::updateChannelControls() {

	boolean channelCVChanged = channelCVInput.update();
	boolean channelPotChanged = channelPotInput.update();

	uint16_t channelChanged = 0;

	if(channelCVChanged || channelPotChanged) {
		int channel = (int) constrain(channelCVInput.currentValue + channelPotInput.currentValue, 0, channelCount - 1);

		if (channel != playState->currentChannel) {
			D(Serial.print("Channel ");Serial.println(channel););
			playState->nextChannel = channel;
			channelChanged |= CHANNEL_CHANGED;
			if((channelPotImmediate && channelPotChanged) || (channelCVImmediate && channelCVChanged)) {
				playState->channelChanged = true;
			}
		};
	}

    return channelChanged;
}

uint16_t Interface::updateStartControls() {
	uint16_t changes = 0;

	boolean cvChanged = startCVInput.update();
	boolean potChanged = startPotInput.update();

	if(potChanged) {
		changes |= TIME_POT_CHANGED;
		if(startPotImmediate) {
			changes |= CHANGE_START_NOW;
		}
	}

	if(cvChanged) {
		changes |= TIME_CV_CHANGED;
		if(startCVImmediate) {
			changes |= CHANGE_START_NOW;
		}
	}

	start = constrain(((startCVInput.currentValue * startCVDivider) + (startPotInput.currentValue * startCVDivider)),0,ADC_MAX_VALUE);

//	if(changes) {
//		D(
//				Serial.print("Start ");
//				Serial.print(start);
//				Serial.print("\t");
//				Serial.print(startCVInput.currentValue);
//				Serial.print("\t");
//				Serial.println(startPotInput.currentValue);
//		);
//		D(startPotInput.printDebug(););
//		D(startCVInput.printDebug(););
//	}
	return changes;
}

// return bitmap of state of changes for CV, Pot and combined Note.
uint16_t Interface::updateRootControls() {

	uint16_t change = 0;

	boolean cvChanged = startCVInput.update();
	boolean potChanged = startPotInput.update();

    // early out if no changes
    if(!cvChanged && !potChanged) {
    	return change;
    }

    float rootPot = startPotInput.currentValue;
    float rootCV = startCVInput.currentValue;

    if(cvChanged) {
    	if(quantiseRootCV) {
        	rootNoteCV = floor(rootCV);
        	if(rootNoteCV != rootNoteCVOld) {
        		change |= ROOT_CV_CHANGED;
        	}
    	} else {
    		rootNoteCV = rootCV;
    		change |= ROOT_CV_CHANGED;
    	}
    }

    if(potChanged) {
    	if(quantiseRootPot) {
        	rootNotePot = floor(rootPot);
        	if(rootNotePot != rootNotePotOld) {
        		change |= ROOT_POT_CHANGED;
        	}
    	} else {
    		rootNotePot = rootPot;
    		change |= ROOT_POT_CHANGED;
    	}
    }

	rootNote = rootNoteCV + rootNotePot;

    // Flag note changes when the note index itself changes
    if(floor(rootNote) != rootNoteOld) {
    	change |= ROOT_NOTE_CHANGED;
    	rootNoteOld = floor(rootNote);
    }

	return change;
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
