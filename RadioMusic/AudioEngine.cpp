#include "AudioEngine.h"
#include "Arduino.h"

#include "RadioMusic.h"

#ifdef DEBUG_ENGINE
#define D(x) x
#else
#define D(x)
#endif

void AudioEngine::init(Settings& config) {

	AudioMemory(25);

	settings = &config;

	playRaw1.loopPlayback(settings->looping);
	playRaw2.loopPlayback(settings->looping);

	mixer.gain(0, 1.0);
	mixer.gain(1, 1.0);

	error = false;
}

D(
void AudioEngine::test(AudioFileInfo& info1, AudioFileInfo& info2) {
	currentPlayer->playFrom(&info1);
	previousPlayer->playFrom(&info2);
}

void AudioEngine::testSwap() {
	pFadeOut->fadeOut(500);
	pFadeIn->fadeIn(500);
	prevAudioElapsed = 0;
	if(pFadeOut == &fade1) {
		pFadeOut = &fade2;
		pFadeIn = &fade1;
	} else {
		pFadeOut = &fade1;
		pFadeIn = &fade2;
	}
}
);

void AudioEngine::swap() {
	// Swap players and fades.
	if (currentPlayer == &playRaw1) {
		currentPlayer = &playRaw2;
		previousPlayer = &playRaw1;
		pFadeOut = &fade1;
		pFadeIn = &fade2;
		D(
		Serial.println("AE: Playing through player 2");
		);
	} else {
		currentPlayer = &playRaw1;
		previousPlayer = &playRaw2;
		pFadeOut = &fade2;
		pFadeIn = &fade1;
		D(
		Serial.println("AE: Playing through player 1");
		);
	}
}

boolean AudioEngine::update() {

	if (currentPlayer->hasFinished()) {
		if(settings->looping) {
			currentPlayer->restart();
			return false;
		} else {
			// Flag EOF
			return true;
		}
	}

	// Check if previously playing rawPlayer has reached fadeOut period and can be stopped.
	// Or if MUTE is not active.
	if ((prevAudioElapsed > settings->crossfadeTime || !settings->crossfade) && previousPlayer->isPlaying()) {
		D(Serial.println("AE: Pause previous player"););
		previousPlayer->stop();
	}

	if(currentPlayer->errors > MAX_ERRORS) {
		D(Serial.println("Player reached max errors."););
		error = true;
	} else if(currentPlayer->errors > 0) {
		D(Serial.print("Error count ");Serial.println(currentPlayer->errors););
	}
	return false;
}

void AudioEngine::changeTo(AudioFileInfo* fileInfo, unsigned long start) {

	D(
		Serial.print("AE: current file is now ");
		Serial.println(fileInfo->name);
		Serial.print("AE: Current player has ");
		Serial.print(currentPlayer->rawfile.name());
		Serial.println();
	);

	uint32_t pos = 0;

	if (settings->looping && currentFileInfo != NULL) {
		D(
			Serial.print("Elapsed ");
			Serial.println(elapsed);
		);
		if(settings->loopMode == LOOP_MODE_RADIO) {
			currentFileInfo->startPlayFrom += ((elapsed * currentFileInfo->getSampleRate() * 2) / 1000);
			if(currentFileInfo->startPlayFrom % currentFileInfo->getBytesPerSample() != 0) {
				currentFileInfo->startPlayFrom -= currentFileInfo->startPlayFrom % currentFileInfo->getBytesPerSample();
			}
			pos = (fileInfo->startPlayFrom + ((elapsed * fileInfo->getSampleRate()) / 1000));
		} else if(settings->loopMode == LOOP_MODE_CONTINUE) {
			pos = currentPlayer->offset() * fileInfo->size;
		} else if(settings->loopMode == LOOP_MODE_START_POINT) {
			pos = (start * fileInfo->size) >> 13;
		}

	} else {
		// set start from arg
		pos = (start * fileInfo->size) >> 13;
	}

	if(pos % fileInfo->getBytesPerSample() != 0) {
		pos -= pos % fileInfo->getBytesPerSample();
	}

	fileInfo->startPlayFrom = pos % fileInfo->size;

	D(
		Serial.print("AE: start play from ");
		Serial.println(fileInfo->startPlayFrom);
	);

	elapsed = 0;
	currentFileInfo = fileInfo;

	// Re-apply speed limits
	if(true) {
		setPlaybackSpeed(currentPlayer->playbackSpeed);
	}

	if(settings->hardSwap) {
		// If we allow all audio file types then no crossfades, just hard
		// cut from one to the next
		AudioNoInterrupts();
		currentPlayer->playFrom(currentFileInfo);   // change audio
		previousPlayer->stop();
		AudioInterrupts();
	} else {

		swap();

		AudioNoInterrupts();
		currentPlayer->playFrom(currentFileInfo);   // change audio

		if (settings->crossfade) {
			// Do a crossfade.
			D(Serial.print("Crossfade ");Serial.println(settings->crossfadeTime););
			pFadeOut->fadeOut(settings->crossfadeTime);
			pFadeIn->fadeIn(settings->crossfadeTime);
			// And reset the fade timer to let the previous file fade out before pausing it.
			prevAudioElapsed = 0;
		} else {
			D(Serial.println("1ms xfade."););
			// Emulate no crossfade with 1ms fadeout/in
			pFadeOut->fadeOut(2);
			pFadeIn->fadeIn(2);
			prevAudioElapsed = 0;
		}
		AudioInterrupts();
	}

	D(
		Serial.print("AE: Current file ");
		Serial.print(currentPlayer->rawfile.name());
		Serial.print(" .Prev file ");
		Serial.print(previousPlayer->rawfile.name());
		Serial.println();
	);

}

void AudioEngine::setPlaybackSpeed(float speed) {
	// Limit speed on high bandwidth audio to not overload CPU
	if(settings->anyAudioFiles && speed > 3.56) {
		if(speed > 3.56) speed = 3.563595;
	} else if(speed > 4.489) {
		speed = 4.4898;
	}
	D(
		Serial.print("AE: Set Playback Speed ");
		Serial.println(speed,6);
		Serial.print("AE: Bandwidth ");
		Serial.print(currentFileInfo->getBandwidth());
		Serial.print(" ");
		Serial.println(currentFileInfo->getSampleRate());
	);
	currentPlayer->playbackSpeed = speed;
	previousPlayer->playbackSpeed = speed;
}

// pos is from 0 -> 8192
void AudioEngine::skipTo(uint32_t pos) {
	uint32_t samplePos = ((float)pos / 8192.0) * (currentFileInfo->size / currentFileInfo->getBytesPerSample());
	D(
		Serial.print("AE: Skip To ");
		Serial.println(samplePos);
	);
	currentFileInfo->startPlayFrom = (samplePos * currentFileInfo->getBytesPerSample()) % currentFileInfo->size;
	currentPlayer->skipTo(currentFileInfo->startPlayFrom);
	elapsed = 0;
}

void AudioEngine::measure() {
	Serial.print("AE: Buffer fills ");
	Serial.print(playRaw1.bufferFills);
	Serial.print("\t");
	Serial.print(playRaw2.bufferFills);
	Serial.print(" .Updates ");
	Serial.print(playRaw1.updates);
	Serial.print("\t");
	Serial.print(playRaw2.updates);
	Serial.print(" .CPU ");
	Serial.print(playRaw1.processorUsageMax());
	Serial.print("\t");
	Serial.print(playRaw2.processorUsageMax());
	Serial.print(" . Errors ");
	Serial.print(playRaw1.errors);
	Serial.print("\t");
	Serial.println(playRaw2.errors);

	playRaw1.bufferFills = 0;
	playRaw2.bufferFills = 0;
	playRaw1.updates = 0;
	playRaw2.updates = 0;
	playRaw1.processorUsageMaxReset();
	playRaw2.processorUsageMaxReset();
}

float AudioEngine::getPeak() {
	if (peak1.available()) {
		return peak1.read();
	} else {
		return 0;
	}
}
