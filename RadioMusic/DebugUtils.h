#ifndef DebugUtils_h
#define DebugUtils_h

#include "Arduino.h"

void showAddress(void* thing) {
	uint16_t ptr;
	char string[]="0123456789ABCDEF";       // array of characters corresponding to numbers from 0 to 15

	/* these lines go inside the loop */
	ptr = (uint16_t) &thing;                // store 16-bit address of 'value'
	Serial.write( string[ (ptr >> 12) & 0xF ] ); // Write out highest 4-bits of memory address
	Serial.write( string[ (ptr >>  8) & 0xF ] );
	Serial.write( string[ (ptr >>  4) & 0xF ] );
	Serial.write( string[ (ptr >>  0) & 0xF ] ); // Write out lowest 4-bits of memory address
}

#endif
