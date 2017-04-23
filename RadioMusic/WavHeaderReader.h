#ifndef WavHeaderReader_h
#define WavHeaderReader_h

#include "SD.h"
#include "AudioFileInfo.h"

class WavHeaderReader {
public:
	// Return true if header was read successfully
	boolean read(File* file, AudioFileInfo& info);
private:
	uint32_t readLong();
	uint16_t readShort();

	File* waveFile;
};

#endif
