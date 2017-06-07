

#include "PlayState.h"

#include "Arduino.h"

void PlayState::printDebug() {
	Serial.print("Play State: ");
	Serial.print(currentChannel);
	Serial.print("\t");
	Serial.print(nextChannel);
	Serial.print("\t");
	Serial.print(channelChanged);
	Serial.print("\t");

}
