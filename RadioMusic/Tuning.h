#ifndef Tuning_h
#define Tuning_h

#include <SD.h>

class Tuning {

	public:
		Tuning(const char* filename);
		boolean init();
		boolean read();
		float pitchValues[128];
		float* createNoteMap();
		float getStandardFreq(float note);
	private:
		const char* _filename;
		float ratios[128];
		int numRatios;
		File scalaFile;
		float processRatio(String* ratioLine);
		boolean addRatio(String* ratioLine);
};

#endif
