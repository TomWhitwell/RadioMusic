#include <SD.h>
#include "Settings.h"

#include "RadioMusic.h"

#ifdef DEBUG_SETTINGS
#define D(x) x
#else
#define D(x)
#endif

Settings::Settings(const char* filename) {
	_filename = filename;
}

void Settings::init(boolean hasSD) {

	if (!hasSD) {
		// Configure defaults
		copyDefaults();
	} else {
		if (SD.exists(_filename)) {
			read();
		} else {
			write();
			read();
		};
	}
}

void Settings::copyDefaults() {

}

void Settings::read() {

	D(Serial.println("Reading settings.txt"););

	char character;
	String settingName;
	String settingValue;
	settingsFile = SD.open("settings.txt");
	if (settingsFile) {
		while (settingsFile.available()) {
			character = settingsFile.read();
			while (character != '=') {
				settingName = settingName + character;
				character = settingsFile.read();
			}
			character = settingsFile.read();
			while (character != '\n') {
				settingValue = settingValue + character;
				character = settingsFile.read();
				if (character == '\n') {
					// Apply the value to the parameter
					applySetting(settingName, settingValue);
					// Reset Strings
					settingName = "";
					settingValue = "";
				}
			}
		}
		// close the file:
		settingsFile.close();
	} else {
		// if the file didn't open, print an error:
		Serial.println("error opening settings.txt");
	}
	// Do test settings here

//	crossfade = true;
//	crossfadeTime = 1000;
//	looping = false;
//	anyAudioFiles = true;
//	hardSwap = true;
//	chanPotImmediate = false;
//	chanCVImmediate = false;
//	quantizeNote = true;
//	startCVImmediate = true;
//	startPotImmediate = true;
//	quantiseRootPot = true;
//	quantiseRootCV = true;
//	pitchMode = true;
//	startCVDivider = 1;

#ifdef TEST_RADIO_MODE
	radioMode();
#elif TEST_DRUM_MODE
	drumMode();
#endif

	if(anyAudioFiles) {
		hardSwap = true;
	}
}

void Settings::drumMode() {
	crossfade=0;
	crossfadeTime=100;
	showMeter=1;
	meterHide=2000;
	chanPotImmediate=1;
	chanCVImmediate=1;
	startPotImmediate=1;
	startCVImmediate=1;
	startCVDivider=1;
	looping=1;
	sort=1;
	pitchMode=1;
	hardSwap = true;
	anyAudioFiles = true;
	radio = false;
}

void Settings::radioMode() {
	crossfade=1;
	crossfadeTime=100;
	showMeter=1;
	meterHide=2000;
	chanPotImmediate=1;
	chanCVImmediate=1;
	startPotImmediate=0;
	startCVImmediate=0;
	startCVDivider=2;
	looping=1;
	sort=1;
	pitchMode=0;
	hardSwap = false;
	anyAudioFiles = false;
	radio = true;
}
/* Apply the value to the parameter by searching for the parameter name
 Using String.toInt(); for Integers
 toFloat(string); for Float
 toBoolean(string); for Boolean
 */
void Settings::applySetting(String settingName, String settingValue) {

	D(
	Serial.print(settingName);
	Serial.print(" -> ");
	Serial.println(settingValue);
	);

	if (settingName.equalsIgnoreCase("mute") || settingName.equalsIgnoreCase("crossfade")) {
		crossfade = toBoolean(settingValue);
	}

	if (settingName.equalsIgnoreCase("declick") || settingName.equalsIgnoreCase("crossfadeTime")) {
		crossfadeTime = settingValue.toInt();
	}

	if (settingName.equalsIgnoreCase("showMeter")) {
		showMeter = toBoolean(settingValue);
	}

	if (settingName.equalsIgnoreCase("meterHide")) {
		meterHide = settingValue.toInt();
	}

	if (settingName.equalsIgnoreCase("chanPotImmediate")) {
		chanPotImmediate = toBoolean(settingValue);
	}

	if (settingName.equalsIgnoreCase("chanCVImmediate")) {
		chanCVImmediate = toBoolean(settingValue);
	}

	if (settingName.equalsIgnoreCase("startPotImmediate")) {
		startPotImmediate = toBoolean(settingValue);
	}

	if (settingName.equalsIgnoreCase("startCVImmediate")) {
		startCVImmediate = toBoolean(settingValue);
	}

	if (settingName.equalsIgnoreCase("startCVDivider")) {
		startCVDivider = settingValue.toInt();
	}

	if (settingName.equalsIgnoreCase("looping")) {
		looping = toBoolean(settingValue);
	}

	if (settingName.equalsIgnoreCase("sort")) {
		sort = toBoolean(settingValue);
	}

	if(settingName.equalsIgnoreCase("anyAudioFiles")) {
		anyAudioFiles = toBoolean(settingValue);
	}

	if(settingName.equalsIgnoreCase("pitchMode")) {
		pitchMode = toBoolean(settingValue);
	}

	if(settingName.equalsIgnoreCase("hardSwap")) {
		hardSwap = toBoolean(settingValue);
	}

	if(settingName.equalsIgnoreCase("noteRange")) {
		noteRange = settingValue.toInt();
	}

	if(settingName.equalsIgnoreCase("radio")) {
		radio = toBoolean(settingValue);
	}

	if(settingName.equalsIgnoreCase("quantiseNoteCV") || settingName.equalsIgnoreCase("quantizeNoteCV")) {
		quantiseRootCV = toBoolean(settingValue);
	}
	if(settingName.equalsIgnoreCase("quantiseNotePot") || settingName.equalsIgnoreCase("quantizeNotePot")) {
		quantiseRootPot = toBoolean(settingValue);
	}

}

// converting string to Float
float Settings::toFloat(String settingValue) {
	char floatbuf[settingValue.length()];
	settingValue.toCharArray(floatbuf, sizeof(floatbuf));
	float f = atof(floatbuf);
	return f;
}

// Converting String to integer and then to boolean
// 1 = true
// 0 = false
boolean Settings::toBoolean(String settingValue) {
	if (settingValue.toInt() == 1) {
		return true;
	} else {
		return false;
	}
}

void Settings::write() {
	Serial.println("Settings file not found, writing new settings");

	// Delete the old One
	SD.remove("settings.txt");
	// Create new one
	settingsFile = SD.open("settings.txt", FILE_WRITE);
	settingsFile.print("crossfade=");
	settingsFile.println(crossfade);
	settingsFile.print("crossfadeTime=");
	settingsFile.println(crossfadeTime);
	settingsFile.print("showMeter=");
	settingsFile.println(showMeter);
	settingsFile.print("meterHide=");
	settingsFile.println(meterHide);
	settingsFile.print("chanPotImmediate=");
	settingsFile.println(chanPotImmediate);
	settingsFile.print("chanCVImmediate=");
	settingsFile.println(chanCVImmediate);
	settingsFile.print("startPotImmediate=");
	settingsFile.println(startPotImmediate);
	settingsFile.print("startCVImmediate=");
	settingsFile.println(startCVImmediate);
	settingsFile.print("startCVDivider=");
	settingsFile.println(startCVDivider);
	settingsFile.print("looping=");
	settingsFile.println(looping);
	settingsFile.print("sort=");
	settingsFile.println(sort);
	settingsFile.print("pitchMode=");
	settingsFile.println(pitchMode);
	// close the file:
	settingsFile.close();
}
