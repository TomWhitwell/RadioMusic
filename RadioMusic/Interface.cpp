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

	// make it backwards compatible with the old 10-bit cv and divider
	startCVDivider = settings.startCVDivider * (ADC_MAX_VALUE / 1024);

	pitchMode = settings.pitchMode;

	switch(pitchMode) {
		default:
		case Settings::PitchMode::Start: {
			D(Serial.print("Set Start Range ");Serial.println(ADC_MAX_VALUE / startCVDivider););
			startPotInput.setRange(0.0, ADC_MAX_VALUE / startCVDivider, false);
			startPotInput.setAverage(true);
			startPotInput.borderThreshold = 32;

			startCVInput.setRange(0.0, ADC_MAX_VALUE / startCVDivider, false);
			startCVInput.setAverage(true);
			startCVInput.borderThreshold = 32;
			break;
		}
		case Settings::PitchMode::Speed: {
			quantiseRootPot = settings.quantiseRootPot;
			startPotInput.setRange(0.0,48, quantiseRootPot);
			startPotInput.borderThreshold = 64;

			quantiseRootCV = settings.quantiseRootCV;
			float lowNote = settings.lowNote + 0.5;
			startCVInput.setRange(lowNote, lowNote + settings.noteRange, quantiseRootCV);
			startCVInput.borderThreshold = 64;
			break;
		}
		case Settings::PitchMode::PotSpeedCvStart: {
			quantiseRootPot = settings.quantiseRootPot;
			startPotInput.setRange(0.0,48, quantiseRootPot);
			startPotInput.borderThreshold = 64;

			startCVInput.setRange(0.0, ADC_MAX_VALUE / startCVDivider, false);
			startCVInput.setAverage(true);
			startCVInput.borderThreshold = 32;

			break;
		}

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
	uint16_t startChanged = updateStartControls();
	uint16_t rootChanged = updateRootControls();

	changes = channelChanged;
	changes |= startChanged;
	changes |= rootChanged;
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
		} else {
			D(
				Serial.print("Channel change flag but channel is the same: ");
				Serial.print(channel);
				Serial.print(" ");
				Serial.print(channelCVInput.currentValue);
				Serial.print(" ");
				Serial.print(channelPotInput.currentValue);
				Serial.print(" ");
				Serial.println(playState->currentChannel);
			);
		}
	}

    return channelChanged;
}

uint16_t Interface::updateStartControls() {

	if(pitchMode == Settings::PitchMode::Speed) {
		return 0;
	}

	uint16_t changes = 0;

	if(pitchMode == Settings::PitchMode::Start) {
		boolean potChanged = startPotInput.update();
		if(potChanged) {
			changes |= TIME_POT_CHANGED;
			if(startPotImmediate) {
				changes |= CHANGE_START_NOW;
			}
		}
	}

	if((pitchMode == Settings::PitchMode::Start) || (pitchMode == Settings::PitchMode::PotSpeedCvStart)) {
		boolean cvChanged = startCVInput.update();
		if(cvChanged) {
			changes |= TIME_CV_CHANGED;
			if(startCVImmediate) {
				changes |= CHANGE_START_NOW;
			}
		}
	}

	switch(pitchMode) {
		default:
		case Settings::PitchMode::Start: {
			start = constrain(((startCVInput.currentValue * startCVDivider) + (startPotInput.currentValue * startCVDivider)),0,ADC_MAX_VALUE);
			break;
		}
		case Settings::PitchMode::PotSpeedCvStart: {
			start = constrain(((startCVInput.currentValue * startCVDivider)),0,ADC_MAX_VALUE);
			break;
		}
	}

	if(changes) {
//		D(
//				Serial.print("Start ");
//				Serial.print(start);
//				Serial.print("\t");
//				Serial.print(startCVInput.currentValue);
//				Serial.print("\t");
//				Serial.println(startPotInput.currentValue);
//		);
		D(startPotInput.printDebug(););
//		D(startCVInput.printDebug(););
	}
	return changes;
}

// return bitmap of state of changes for CV, Pot and combined Note.
uint16_t Interface::updateRootControls() {

	if (pitchMode == Settings::PitchMode::Start) {
		return 0;
	}

	uint16_t change = 0;

	if(pitchMode == Settings::PitchMode::Speed) {
		boolean cvChanged = startCVInput.update();
		if(cvChanged) {
			D(Serial.println("CV Changed"););
			float rootCV = startCVInput.currentValue;
			if(quantiseRootCV) {
				rootNoteCV = floor(rootCV);
				if(rootNoteCV != rootNoteCVOld) {
					D(Serial.print("CV ");Serial.println(startCVInput.inputValue););
					change |= ROOT_CV_CHANGED;
				}
			} else {
				rootNoteCV = rootCV;
				change |= ROOT_CV_CHANGED;
			}
		}
	}

	if((pitchMode == Settings::PitchMode::Speed) || (pitchMode == Settings::PitchMode::PotSpeedCvStart)) {
		boolean potChanged = startPotInput.update();
		if(potChanged) {
			D(Serial.println("Pot Changed"););
			float rootPot = startPotInput.currentValue;
			if(quantiseRootPot) {
				rootNotePot = floor(rootPot);
				if(rootNotePot != rootNotePotOld) {
					D(Serial.print("Pot ");Serial.println(startPotInput.inputValue););
					change |= ROOT_POT_CHANGED;
				}
			} else {
				rootNotePot = rootPot;
				change |= ROOT_POT_CHANGED;
			}
		}
	}

	if(!change)
		return 0;

	switch(pitchMode) {
    default:
		case Settings::PitchMode::Speed: {
			rootNote = rootNoteCV + rootNotePot;
			break;
		}
		case Settings::PitchMode::PotSpeedCvStart: {
			rootNote = 24 + rootNotePot;
			break;
		}
	}

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
