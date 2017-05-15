/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef RMPlaySDraw_h_
#define RMPlaySDraw_h_

#include "AudioStream.h"
#include <SD.h>
#include "AudioFileInfo.h"

// Audio system AUDIO_BLOCK_SAMPLES is 128 and runs at 44k 16bit
// This means we need 2 BLOCKS of bytes for a block 44k 16
// 3 for 44k 24bit
// 6.54 for 96k 24bit
// TODO : Use 2 buffers. 1 for multiples of 44k and 1 for multiples of 48k
// If there is a reasonable buffer size which is byte aligned at 48k
// e.g. an 836 byte buffer would have enough samples to fill 3 blocks for 48/16 -> 44/16
// NOTE : Make sure this is a factor of 2 AND 3 otherwise 24-bit samples will be split
// across the buffer boundary and that will make things go wrong.
#define AUDIOBUFSIZE (AUDIO_BLOCK_SAMPLES * 18)

class SDPlayPCM: public AudioStream {
public:
	SDPlayPCM(void) : AudioStream(0, NULL) { begin(); }	
	void begin(void);
	bool playFrom(AudioFileInfo* info);
	bool hasFinished(void) { return finished; }
	void restart();
	bool isPlaying(void) { return playing; }
	void stop(void);
	void loopPlayback(bool loop);

	bool skipTo(uint32_t dataOffset);
	float offset(void);

	virtual void update(void);

	uint8_t playerID = 0;

	volatile bool readError = false;
	volatile float playbackSpeed = 1.0;

	volatile uint16_t bufferFills = 0;
	volatile uint16_t updates = 0;
	volatile uint16_t errors = 0;

	File rawfile;
	volatile bool inUpdate = false;
	volatile bool updateRequired = false;

private:
	bool changeFileTo(AudioFileInfo* info, bool closeFirst);

	bool fillBuffer(int32_t requiredBytes);
	void fastFillBuffer();
	void debugHeader();
	uint32_t fileAvailable();

	// audioBuffer is a ring buffer
	unsigned char audioBuffer[AUDIOBUFSIZE];

	// Speed to play different SRs at normal speed.
	// 44100 is 1.0
	volatile float sampleRateSpeed = 1.0;
	// Playing speed independent of sample rate.
	// 1.0 is always original speed
	volatile float speed = 1.0;
	volatile bool playing;
	volatile bool finished;
	volatile bool looping;
	volatile int bytesPerSample = 2;
	volatile int32_t bytesAvailable = 0;
	volatile int32_t readPositionInBytes = 0;
	volatile uint32_t bufferFillPosition = 0;

	String filename;
	uint32_t dataSize = 0;
	uint32_t dataOffset = 0;
	uint16_t channels = 1;

	int32_t bytesLeftInFile = 0;

	uint32_t l0 = 0;
	uint32_t lowSamplePos = 0;
	uint32_t bytesUsed = 0;
	uint16_t spaceLeftInBuffer = 0;
	int32_t bytesRequired = 0;
	int32_t read = 0;

//	// Fixed point phase. 5 bit integer 11 bit fractional
//	// This is phase in samples, not bytes.
//	uint32_t phaseIncrement = 0;
};

#endif
