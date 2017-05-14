#ifndef AudioEngine_h
#define AudioEngine_h

#include <Audio.h>

#include "SDPlayPCM.h"
#include "Settings.h"

#define MAX_ERRORS 10

class AudioEngine {
	public:
		void test(AudioFileInfo& info1, AudioFileInfo& info2);
		void testSwap();

		SDPlayPCM *currentPlayer;
		SDPlayPCM *previousPlayer;
		AudioEffectFade *pFadeOut;
		AudioEffectFade *pFadeIn;

		// Audio engine definitions.
		SDPlayPCM playRaw1;
		SDPlayPCM playRaw2;
		AudioEffectFade fade1;
		AudioEffectFade fade2;
		AudioMixer4 mixer;
		AudioAnalyzePeak peak1;
		AudioOutputAnalog dac1;
		AudioConnection patchCord1;
		AudioConnection patchCord2;
		AudioConnection patchCord3;
		AudioConnection patchCord4;
		AudioConnection patchCord5;
		AudioConnection patchCord6;

		elapsedMillis prevAudioElapsed;
		boolean eof = false;

		AudioEngine() :
			patchCord1(playRaw1, fade1),
			patchCord2(playRaw2, fade2),
			patchCord3(fade1, 0, mixer, 0),
			patchCord4(fade2, 0, mixer, 1),
			patchCord5(mixer, 0, dac1, 0),
			patchCord6(mixer, 0, peak1, 0){

			playRaw1.playerID = 1;
			playRaw2.playerID = 2;
			currentPlayer = &playRaw1;

			previousPlayer = &playRaw2;
			pFadeOut = &fade1;
			pFadeIn = &fade2;
		}

		void init(Settings& settings);
		boolean update();
		void swap();
		void changeTo(AudioFileInfo* audioFileInfo, unsigned long start);
		void printDebug();
		void skipTo(uint32_t time);
		void setPlaybackSpeed(float speed);
		void measure();
		float getPeak();
		AudioFileInfo* currentFileInfo;
		boolean error = false;

		// Time elapsed since last switch / skip
		elapsedMillis elapsed = 0;

	private:
		uint16_t waitCount = 0;
		boolean updateRequired = false;
		AudioFileInfo* nextInfo;
		Settings* settings;

//		boolean hardSwap = false;
//		boolean looping = false;
//		boolean mute = false;
//		uint16_t declick = 0;

};

#endif
