#ifndef Settings_h
#define Settings_h

#include <SD.h>

class Settings {
public:
	Settings(const char* filename);
	void init(boolean hasSD);
	void read();
	void write();
	float toFloat(String settingValue);
	boolean toBoolean(String settingValue);

	uint16_t declick = 25; // milliseconds of fade in/out on switching
	uint16_t meterHide = 2000; // how long to show the meter after bank change in Milliseconds
	uint16_t startCVDivider = 2; // Changes sensitivity of Start control. 1 = most sensitive 512 = least sensitive (i.e only two points)
	boolean mute = false; // Crossfade clicks when changing channel / position at cost of speed. Fade speed is set by DECLICK
	boolean showMeter = true; // Does the VU meter appear?

	boolean chanPotImmediate = true; // Settings for Pot / CV response.
	boolean chanCVImmediate = true; // TRUE means it jumps directly when you move or change.

	boolean startPotImmediate = false; // FALSE means it only has an effect when RESET is pushed or triggered
	boolean startCVImmediate = false;

	boolean looping = false; // When a file finishes, start again from the beginning

	boolean sortFiles = true; // By default we sort the directory contents.

	boolean gotSort = false; // true if settings.txt exists and contains Sort setting.
    // We don't want to turn on sorting on disks that
    // already have settings.txt, the user might have
    // done efforts to do fat sorting on it.

	// Use start pot and cv to control speed instead of start point
	boolean speedControl = false;

	// If this is true we'll read any .wav files and try to play them
	// if not we'll only play 44khz, 16bit mono files
	boolean anyAudioFiles = true;

	// Use reset CV as an output
	boolean resetIsOutput = false;

	// Only overrides if true. Force no crossfades.
	boolean hardSwap = false;
private:
	const char* _filename;
	File settingsFile;
	void copyDefaults();
	void applySetting(String name, String value);
};

#endif
