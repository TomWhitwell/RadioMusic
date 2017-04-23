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

	mixer.gain(0, 0.4);
	mixer.gain(1, 0.4);

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
	if ((prevAudioElapsed > settings->declick || !settings->mute) && previousPlayer->isPlaying()) {
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

void AudioEngine::changeTo(AudioFileInfo* fileInfo) {

	currentFileInfo = fileInfo;

	D(
		Serial.print("AE: current file is now ");
		Serial.println(currentFileInfo->name);
		Serial.print("AE: Current player has ");
		Serial.print(currentPlayer->rawfile.name());
		Serial.println();
	);

	if (!eof && settings->looping) {
		// Carry on from previous position, unless reset pressed or time selected
		// Make this file play from a new offset but ensure its aligned to samples
		// Get the current playback position of the active player
		uint32_t playbackPos = (currentFileInfo->size * currentPlayer->offset()) / 4096;
		if(playbackPos % currentFileInfo->getBytesPerSample() != 0) {
			playbackPos -= playbackPos % currentFileInfo->getBytesPerSample();
		}
		currentFileInfo->startPlayFrom = playbackPos;
		// Move back to nearest sample
		D(
			Serial.print("AE: No eof, No reset. Play from is ");
			Serial.println(currentFileInfo->startPlayFrom);
		);
	}

	// The file is marked for reaching end of file.
	// Start from 0 but preserve playhead for future resets.
	if (eof) {
		D(
			Serial.println("AE: got EOF");
		);
		currentFileInfo->startPlayFrom = 0;
	} else {
		D(
			Serial.print("AE: Resuming playback from ");
			Serial.println(currentFileInfo->startPlayFrom);
		);
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

		if (settings->mute) {
			// Do a crossfade.
			D(Serial.print("Crossfade ");Serial.println(settings->declick););
			pFadeOut->fadeOut(settings->declick);
			pFadeIn->fadeIn(settings->declick);
			// And reset the fade timer to let the previous file fade out before pausing it.
			prevAudioElapsed = 0;
		} else {
			D(Serial.println("1ms xfade."););
			// Emulate no crossfade with 1ms fadeout/in
			pFadeOut->fadeOut(1);
			pFadeIn->fadeIn(1);
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

	eof = false;
}

void AudioEngine::setPlaybackSpeed(float speed) {
//	Serial.print("Set Playback Speed ");
//	Serial.println(speed,4);
	currentPlayer->playbackSpeed = speed;
	previousPlayer->playbackSpeed = speed;
}

// pos is from 0 -> 8192
void AudioEngine::skipTo(uint32_t pos) {
		uint32_t samplePos = ((float)pos / 8192.0) * (currentFileInfo->size / currentFileInfo->getBytesPerSample());
//		D(
//			Serial.print("AE: Skip To ");
//			Serial.println(samplePos);
//		);
		currentFileInfo->startPlayFrom = (samplePos * currentFileInfo->getBytesPerSample()) % currentFileInfo->size;
		currentPlayer->skipTo(currentFileInfo->startPlayFrom);
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
