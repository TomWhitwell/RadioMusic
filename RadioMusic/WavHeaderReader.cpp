#include "WavHeaderReader.h"

#include <Audio.h>
#include <SD.h>

#include "RadioMusic.h"

#ifdef DEBUG_WAV
#define D(x) x
#else
#define D(x)
#endif

boolean WavHeaderReader::read(File* file, AudioFileInfo& info) {

	waveFile = file;

	if (waveFile->available()) {
		D(
			Serial.print("Bytes available ");
			Serial.println(waveFile->available());
		);
		uint32_t chunkSize = 0;

		// 'RIFF'
		if (!waveFile->seek(4)) {
			D(Serial.println("Seek past RIFF failed"); );
			return false;
		} else {
			D(Serial.print("Cur Pos "); Serial.println(waveFile->position()););
		}

		// File Size 4 bytes
		uint32_t fileSize = readLong();
		D(Serial.print("File size "); Serial.println(fileSize););
		// 'WAVE' as little endian uint32 is 1163280727
		// If chunk ID isnt 'WAVE' stop here.
		if (readLong() != 1163280727) {
			D(Serial.println("Chunk ID not WAVE"); );
			return false;
		}

		uint32_t nextID = readLong();
		// Keep skipping chunks until we hit 'fmt '
		while (nextID != 544501094) {
			chunkSize = readLong();
			D(Serial.print("Skipping "); Serial.println(chunkSize););
			waveFile->seek(waveFile->position() + chunkSize);
			nextID = readLong();
		}

		if (!waveFile->available()) {
			D(Serial.println("Skipped whole file"); );
			return false;
		}

		D(Serial.print("Found fmt chunk at "); Serial.println(waveFile->position()););
		// Next block is fmt sub chunk
		// subchunk ID : 'fmt ' (note the space) 4 bytes
		// fmt subchunk Size : 4 bytes.
		chunkSize = readLong();
		D(Serial.print("Format Chunk Size "); Serial.println(chunkSize););
		// Audio Format 2 bytes : 1 = PCM
		uint16_t format = readShort();

		// NumChannels 2 bytes
		info.setChannels(readShort());

		D(Serial.print("Is Stereo : "); Serial.println(info.format & STEREO););
		// SampleRate 4 bytes
		info.setSampleRate(readLong());

		// ByteRate 4 bytes
		//info.byteRate = readLong();
		uint32_t byteRate = readLong();

		// BlockAlign 2 bytes
		uint16_t blockAlign = readShort();

		// BitsPerSample 2 bytes
		uint16_t bitsPerSample = readShort();
		if (bitsPerSample % 8 != 0) {
			D(Serial.print("Unsupported bit depth "); Serial.println(bitsPerSample););
			return false;
		}
		info.setBitsPerSample(bitsPerSample);

		if (chunkSize > 16) {
			D(Serial.print("Format chunk is extended "); Serial.println(chunkSize););
			waveFile->seek(waveFile->position() + chunkSize - 16);
		}

		// 'data' as little endian uint32 is 1635017060
		// read the chunk ID as a uint32 rather than doing strcmp
		while (readLong() != 1635017060) {
			chunkSize = readLong();
			waveFile->seek(waveFile->position() + chunkSize);
		}

		chunkSize = readLong();
		D(Serial.print("WAV data length "); Serial.println(chunkSize););
		info.size = chunkSize;
		info.dataOffset = waveFile->position();
	} else {
		D(Serial.println("File not available"); );
	}
	return true;
}

uint16_t WavHeaderReader::readShort() {
	uint16_t val = waveFile->read();
	val = waveFile->read() << 8 | val;
	return val;
}

// Wav files are little endian
uint32_t WavHeaderReader::readLong() {

	int32_t val = waveFile->read();
	if (val == -1) {
		D(Serial.println("Long read error. 1"); );
	}
	for (byte i = 8; i < 32; i += 8) {
		val = waveFile->read() << i | val;
		if (val == -1) {
			D(Serial.print("Long read error "); Serial.println(i););
		}
	}
	return val;
}
