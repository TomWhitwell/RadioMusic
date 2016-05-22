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

#include "RMPlaySDRaw.h"
#include "spi_interrupt.h"

void RMPlaySDRaw::begin(void)
{
	playing = false;
	file_offset = 0;
	file_size = 0;
	_filePath[0] = 0;
}


bool RMPlaySDRaw::play(const char *filename)
{
	stop();
	strcpy(_filePath, filename);
	AudioStartUsingSPI();
	__disable_irq();
	rawfile = SD.open(filename);
	__enable_irq();
	if (!rawfile) {
		//Serial.println("unable to open file");
		return false;
	}
	file_size = rawfile.size();
	file_offset = 0;
	//Serial.println("able to open file");
	playing = true;
	return true;
}

bool RMPlaySDRaw::preparePlayFrom(const char *filename)
{
    if (strcmp(_filePath, filename) == 0)
	return true; // Already prepared for this file.
    strcpy(_filePath, filename);
    __disable_irq();
    //Serial.println("pause() - Closing file.");
    rawfile.close();
    rawfile = SD.open(filename);
    __enable_irq();
    if (!rawfile) {
	return false;
    }
    file_size = rawfile.size();
    file_offset = 0;
    return true;
}

bool RMPlaySDRaw::playFrom(const char *filename, unsigned long startPoint)
{
    // We use the same file, just seek inside it.
    if (strcmp(_filePath, filename) == 0) {
	Serial.print("Continuing on file ");
	Serial.println(filename);
	rawfile.seek(startPoint % file_size);
	file_offset = startPoint;
//	    AudioStartUsingSPI();
	playing = true;
	return true;
    }
    stop();
    strcpy(_filePath, filename);
    AudioStartUsingSPI();
    __disable_irq();
    rawfile = SD.open(filename);
    __enable_irq();
    if (!rawfile) {
	Serial.println("NO: unable to open file");
	return false;
    }
    file_size = rawfile.size();
    rawfile.seek(startPoint % file_size);
    file_offset = startPoint;

    Serial.println("YES: able to open file");
    playing = true;
    return true;
}



void RMPlaySDRaw::pause(void) {
    __disable_irq();
    if (playing) {
	playing = false;
//	AudioStopUsingSPI();
    } 
    __enable_irq();
}

void RMPlaySDRaw::stop(void)
{
    __disable_irq();
    //Serial.println("stop() - Closing file.");
    rawfile.close();
    if (playing) {
	playing = false;
	__enable_irq();
	AudioStopUsingSPI();
    } else {
	__enable_irq();
    }
}


void RMPlaySDRaw::update(void)
{
    unsigned int i;
    int n;
    audio_block_t *block;

    // only update if we're playing
    if (!playing) return;

    // allocate the audio blocks to transmit
    block = allocate();
    if (block == NULL) 
	return;

    if (rawfile.available()) {
	// we can read more data from the file...
	if (bufAvail == 0) {
	    bufAvail = rawfile.read(audioBuffer, AUDIOBUFSIZE);
	    bufPos = 0;
	    if (bufAvail < 0) {
		if (hotswap_cb)
		    hotswap_cb();
		return;
	    }
	}
	n = min(AUDIO_BLOCK_SAMPLES * 2, bufAvail);
	memcpy(block->data, &(audioBuffer[bufPos]), n);
	bufAvail -= n;
	bufPos += n;
	// ADD THIS SECTION TO ENABLE HOT SWAPPING 
	// read returns -1 on error.
	// END OF NEW HOT SWAP SECTION

	file_offset += n;
	for (i=n/2; i < AUDIO_BLOCK_SAMPLES; i++) {
	    block->data[i] = 0;
	}
	transmit(block);
    } else {
//	rawfile.close();
//	AudioStopUsingSPI();
	playing = false;
    }
    release(block);
}

#define B2M (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT / 2.0) // 97352592

uint32_t RMPlaySDRaw::positionMillis(void)
{
	return ((uint64_t)file_offset * B2M) >> 32;
}

uint32_t RMPlaySDRaw::lengthMillis(void)
{
	return ((uint64_t)file_size * B2M) >> 32;
}

uint32_t RMPlaySDRaw::fileOffset(void)
{
	return file_offset;
}

