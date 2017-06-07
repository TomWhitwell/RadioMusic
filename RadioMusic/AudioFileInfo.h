#ifndef AudioFileInfo_h
#define AudioFileInfo_h

#include "Arduino.h"

#define CHANNELS_MASK B00000001
#define MONO 		  0
#define STEREO 		  1

#define BIT_DEPTH_MASK 	B00000110
#define BIT_DEPTH_8 	0 // 00
#define BIT_DEPTH_16 	1 // 01
#define BIT_DEPTH_24 	2 // 10
#define BIT_DEPTH_32 	3 // 11

#define SAMPLE_RATE_MASK 	B00111000
#define SAMPLE_RATE_11		0 // 000
#define SAMPLE_RATE_22  	1 // 001
#define SAMPLE_RATE_44  	2 // 010
#define SAMPLE_RATE_48  	3 // 011
#define SAMPLE_RATE_96  	4 // 100

static uint32_t SAMPLE_RATES[5] = { 11025,22050,44100,48000,96000 };
static uint8_t BIT_DEPTHS[4] = {8,16,24,32};

class AudioFileInfo {
public:

	String name;
	// Size doesn't include the header for wav files, just the length of the audio
	uint32_t size;

//	// Only accept 11025, 22050, 44100, 88,200, 48000 and 96000
//	uint32_t sampleRate;
//
//	// Only allow mono or stereo
//	uint16_t channels;
//
//	// Only allow 8, 16, 24 and 32 bit data
//	uint8_t bytesPerSample;

	// sampleRate * channels * bitDepth/8
//	uint32_t byteRate;

	// For wav files where the audio data starts
	uint32_t dataOffset = 0;

	// Store where playback ended if we want to resume
	uint32_t startPlayFrom = 0;

	void setChannels(uint8_t channels) {
		if(channels == 2) {
			format |= 1;
		}
	}

	uint16_t getChannels() {
		return format & STEREO ? 2 : 1;
	}

	// Return false is sample rate not supported
	boolean setSampleRate(uint32_t sampleRate) {
		if(sampleRate % 11025 == 0) {
			uint8_t m = (sampleRate / 11025) - 1;

			if(m == 3) m = 2;
			format |= m << 3;
		} else if(sampleRate % 48000 == 0) {
			if(sampleRate / 48000 == 1) {
				format |= 3 << 3;
			} else {
				format |= 4 << 3;
			}
		} else {
			return false;
		}
		return true;
	}

	uint32_t getSampleRate() {
		return SAMPLE_RATES[(format & SAMPLE_RATE_MASK) >> 3];
	}

	void setBitsPerSample(uint8_t bits) {
		format |= ((bits >> 3) - 1) << 1;
	}

	uint8_t getBytesPerSample() {
		return BIT_DEPTHS[(format & BIT_DEPTH_MASK) >> 1] >> 3;
	}

	// Bytes per second
	uint32_t getBandwidth() {
		return getSampleRate() * getBytesPerSample() * getChannels();
	}

	// Packed format, Big endian.
	// Bit 0 Mono / Stereo
	// Bits 1 + 2 : Bit depth
	// 0 0 : 8 Bit
	// 0 1 : 16 bit
	// 1 0 : 24 bit
	// 1 1 : 32 bit
	// Bits 2 -> 5 : Sample Rate. 11, 22, 44, 48 and 96
	// 0 0 1 : 11
	// 0 1 0 : 22
	// 0 1 1 : 44
	// 1 0 0 : 48
	// 1 0 1 : 96
	uint8_t format = 0;
};

#endif
