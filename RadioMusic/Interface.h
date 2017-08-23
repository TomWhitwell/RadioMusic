#ifndef Interface_h
#define Interface_h

#include "Arduino.h"

#include <Bounce2.h>
#include "PlayState.h"
#include "Settings.h"
#include "AnalogInput.h"

#define CHAN_POT_PIN A9 	// pin for Channel pot
#define CHAN_CV_PIN A6 		// pin for Channel CV
#define TIME_POT_PIN A7 	// pin for Time pot
#define TIME_CV_PIN A8 		// pin for Time CV

// extra values for expander 

#define EX1_POT_PIN A2 // Pin for Expander pot 1 
#define EX1_CV_PIN A4  // Pin for Expander CV 1 
#define EX2_POT_PIN A3  // Pin for Expander pot 2 
#define EX2_CV_PIN A5  // Pin for Expander CV 2 





#define RESET_BUTTON 8 		// Reset button
#define RESET_CV 9 		// Reset pulse input

#define TIME_POT_CHANGED 	1
#define TIME_CV_CHANGED 	1 << 1
#define CHANNEL_CHANGED 	1 << 2
#define CHANGE_START_NOW 	1 << 3
#define BUTTON_SHORT_PRESS 	1 << 4
#define BUTTON_LONG_PRESS 	1 << 5
#define BUTTON_LONG_RELEASE 1 << 6
#define BUTTON_PULSE 		1 << 7
#define RESET_TRIGGERED 	1 << 8
#define ROOT_CV_CHANGED		1 << 9
#define ROOT_POT_CHANGED	1 << 10
#define ROOT_NOTE_CHANGED	1 << 11

#define SHORT_PRESS_DURATION 10
#define LONG_PRESS_DURATION 600
// after LONG_PRESS_DURATION every LONG_PRESS_PULSE_DELAY milliseconds the update
// function will set BUTTON_PULSE
#define LONG_PRESS_PULSE_DELAY 600

#define SAMPLEAVERAGE   16 	// How many values are read and averaged of pot/CV inputs each interface check.

class Interface {
public:
	boolean quantiseRootCV = true;
	boolean quantiseRootPot = true;

	float rootNoteCV = 36;
	float rootNotePot = 36;
	float rootNote = 36;

	boolean buttonHeld = false;
	unsigned long start = 0;

	elapsedMillis buttonHoldTime;

	PlayState* playState;

	Interface() : channelCVInput(CHAN_CV_PIN),
			channelPotInput(CHAN_POT_PIN),
			startCVInput(TIME_CV_PIN),
			startPotInput(TIME_POT_PIN), 
     ex1PotInput(EX1_POT_PIN) 
     

	{
		playState = NULL;
	}

	void init(int fileSize, int channels, const Settings& settings, PlayState* state);
	void setChannelCount(uint16_t count);
	void setFileSize(uint32_t fileSize);

	uint16_t update();
	uint16_t updateButton();
private:
	AnalogInput channelCVInput;
	AnalogInput channelPotInput;
	AnalogInput startCVInput;
	AnalogInput startPotInput;
  AnalogInput ex1PotInput;

	Bounce resetButtonBounce;
	elapsedMillis buttonTimer = 0;

	uint16_t changes = 0;

	uint16_t startCVDivider = 1;
	boolean channelPotImmediate = true;
	boolean channelCVImmediate = true;
	boolean startPotImmediate = true;
	boolean startCVImmediate = true;

	uint16_t channelCount = 0;
	float rootNoteOld = -100;
	float rootNotePotOld = -100;
	float rootNoteCVOld = -100;

	uint16_t updateChannelControls();
	uint16_t updateStartControls();
	uint16_t updateRootControls();
	boolean pitchMode = false;
};

#endif
