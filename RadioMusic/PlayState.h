#ifndef PlayState_h
#define PlayState_h

#include "Arduino.h"

class PlayState {
public:
	uint8_t	bank = 0;
	uint8_t currentChannel = 0;
	uint8_t nextChannel = 0;
	boolean channelChanged = true;

	void printDebug();
};

#endif
