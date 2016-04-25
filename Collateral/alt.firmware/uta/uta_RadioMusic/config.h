#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MAX_FILES 		75
#define BANKS 			16
#define LED0 			6
#define LED1 			5
#define LED2 			4
#define LED3 			3
#define CHAN_POT_PIN 	A9 // pin for Channel pot
#define CHAN_CV_PIN 	A6 // pin for Channel CV 
#define TIME_POT_PIN 	A7 // pin for Time pot
#define TIME_CV_PIN 	A8 // pin for Time CV

#define HOLDTIME 		400
#define BANK_SAVE 		0

extern AudioPlaySdRaw playRaw1;
extern File settingsFile;
extern String FILE_TYPE;
extern String CURRENT_DIRECTORY;
extern int ACTIVE_BANKS; 
extern String FILE_NAMES [BANKS][MAX_FILES];
extern int FILE_COUNT[BANKS];
extern unsigned long FILE_SIZES[BANKS][MAX_FILES];
extern String FILE_DIRECTORIES[BANKS][MAX_FILES];

extern int PLAY_CHANNEL; 
extern int NEXT_CHANNEL; 
extern char* charFilename;
extern int PLAY_BANK;

extern AudioPlaySdRaw playRaw1;
extern AudioAnalyzePeak peak1;

extern boolean MUTE;
extern int DECLICK;
extern boolean ShowMeter;
extern int meterHIDE;
extern boolean ChanPotImmediate;
extern boolean ChanCVImmediate;
extern boolean StartPotImmediate;
extern boolean StartCVImmediate;
extern int StartCVDivider;
extern elapsedMillis fps;
extern boolean Looping;
extern int sampleAverage;

extern int chanPotOld;
extern int chanCVOld;
extern int timPotOld;
extern int timCVOld;

extern int chanHyst;
extern int timHyst;

extern boolean CHAN_CHANGED;
extern unsigned long playhead;
extern boolean RESET_CHANGED;
extern elapsedMillis bankTimer;
extern elapsedMillis meterDisplay;
extern elapsedMillis resetLedTimer;
extern Bounce resetSwitch;

/* channel are selected with knob */
extern boolean bank_select;
#endif