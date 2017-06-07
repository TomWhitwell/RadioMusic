#include <SD.h>
#include "Tuning.h"

//#define DEBUG_TUNING

Tuning::Tuning(const char* filename) {
	_filename = filename;
	numRatios = 0;
}

boolean Tuning::init() {
	boolean exists = SD.exists(_filename);
#ifdef DEBUG_TUNING
	Serial.print("Checking for tuning file ");
	Serial.println(_filename);
	Serial.print("Exists ");
	Serial.println(exists);
#endif
	if(exists) {
		return read();
	} else {
		return false;
	}
}

// Return true if we successfully read a tuning file, false otherwise
boolean Tuning::read() {

	scalaFile = SD.open(_filename);

	char character;

	boolean inComment = false;

	int numEntries = 0;
	ratios[0] = 1.0;
	numRatios = 1;

	int DESCRIPTION_STATE = 1;
	int ENTRIES_STATE = 2;
	int RATIO_STATE = 3;

	int state = DESCRIPTION_STATE;

	String currentLine = "";

	String description;

    while (scalaFile.available()) {
        character = scalaFile.read();

        // If we're in a comment just keep reading until we hit the next line
        if(inComment) {
        	if(character == '\n') {
        		inComment = false;
        	}
        	continue;
        }
        if(character == '!') {
        	// Line is a comment, ignore until end
        	inComment = true;
        	if(state == RATIO_STATE && currentLine.length() > 1) {
        		// If a comment has started on a ratio line, we can process the ratio
        		if(addRatio(&currentLine)) {
        			break;
        		}
        		currentLine = "";
        	}
        	continue;
        } else {
        	inComment = false;
        }

        if(state == DESCRIPTION_STATE) {
        	if(character == '\n') {
        		state = ENTRIES_STATE;
        		if(currentLine.length() == 0) {
        			description = "No Info";
        		} else {
        			description = currentLine;
        		}
				#ifdef DEBUG_TUNING
        		Serial.println("End description");
        		Serial.println(description);
				#endif

        		currentLine = "";
        	} else {
        		currentLine += character;
        	}
        } else if(state == ENTRIES_STATE) {
        	if(character == '\n') {
        		state = RATIO_STATE;
        		numEntries = currentLine.toInt();
        		if(numEntries == 0) {
        			return false;
        		}
				#ifdef DEBUG_TUNING
        		Serial.print("End num entries: ");
        		Serial.println(numEntries);
				#endif

        		currentLine = "";
        	} else {
        		currentLine += character;
        	}
        } else if(state == RATIO_STATE) {
        	if(character == '\n') {
				#ifdef DEBUG_TUNING
        		Serial.print("Got ratio");
        		Serial.println(currentLine);
				#endif

        		if(addRatio(&currentLine)) {
        			break;
        		}
        		currentLine = "";
        	} else {
        		currentLine += character;
        	}
        }
    }

    scalaFile.close();

	#ifdef DEBUG_TUNING
    if(numEntries != numRatios) {
    	Serial.print("Entries and Ratio Count not equal. ");
    	Serial.print(numEntries);
    	Serial.print(" vs ");
    	Serial.println(numRatios);
    }
	#endif

    if(numRatios > 1) {
    	return true;
    }

    return false;
}

// Returns boolean to indicate stopping condition has been met.
// return false means keep adding, return true means dont add any more ratios
boolean Tuning::addRatio(String* ratioText) {
	float r = processRatio(ratioText);
	Serial.print("Add ratio ");
	Serial.print(numRatios);
	Serial.print(" -> ");
	Serial.println(r,4);

	ratios[numRatios] = r;
	if(r < 2.0) {
		numRatios++;
	} else {
		// At or over the octave, stop here.
		return true;
	}

	if(numRatios > 127) {
		Serial.println("WARN. Enough ratios. Discarding extras. Octave scaling is probably broken");
		return true;
	}
	return false;
}
/**
 * Valid Pitch Lines

	81/64
	408.0
	408.
	5
	-5.0
	10/20
	100.0 cents
	100.0 C#
	5/4   E\

	Note : They can also have comments placed after them.
 */
float Tuning::processRatio(String* ratioText) {
	float ratio = 0.0;

	if(ratioText->indexOf(".") > 0) {
		int spacePos = ratioText->indexOf(" ");
		float cents = 0.0;
		if(spacePos > 0) {
			String centsSubstring = ratioText->substring(0,spacePos);

#ifdef DEBUG_TUNING
			Serial.print("Cents Substring:");
			Serial.print(centsSubstring);
			Serial.println(":");
#endif

			cents = centsSubstring.toFloat();
		} else {
			cents = ratioText->toFloat();
		}

		ratio = pow(2,(cents/100.0)/12.0);

#ifdef DEBUG_TUNING
		Serial.print("Ratio is cents ");
		Serial.println(cents);
#endif

	} else if(ratioText->indexOf("/") > 0) {
		int slashPos = ratioText->indexOf("/");
		float numerator = ratioText->substring(0,slashPos).toFloat();
		float denominator = ratioText->substring(slashPos+1).toFloat();
		if(numerator != 0 && denominator != 0) {
			ratio = numerator / denominator;
		}

#ifdef DEBUG_TUNING
		Serial.print("Ratio is ratio ");
		Serial.print(numerator);
		Serial.print("/");
		Serial.print(denominator);
		Serial.print(" = ");
		Serial.println(ratio,4);
#endif

	} else {
		ratio = ratioText->toFloat();
//		Serial.println("Ratio is float");
	}
//	Serial.println(ratio);
	return ratio;
}

float* Tuning::createNoteMap() {

	#ifdef DEBUG_TUNING
	Serial.print("Create note map for ");
	Serial.print(numRatios);
	Serial.println(" ratios.");
	Serial.println("i\tRel\t\tOct\t\tIndex\t\tBase\t\tRatio\t\tFreq");
	#endif

	if(numRatios > 2) {
		int centerNote = 69;
		float centerFrequency = 440.0;
		// Whilst this looks bad, the value at this position is the final octave scaling value
		// from the tuning file
		float octaveSize = ratios[numRatios];
		if(octaveSize <= 0.00000001) {
			octaveSize = 1.0;
			Serial.println("Octave ratio was zero, forcing to 1.");
		}
#ifdef DEBUG_TUNING
		Serial.print("Octave Size ");
		Serial.println(octaveSize,8);
#endif
		int notesPerOctave = numRatios;

		float octaveBaseFreq = 0.0;
		int indexInOctave = 0;
		float noteRatio = 0.0;
		int octavesAway = 0;
		float octaveFactor;
		int relativeNoteIndex = 0;
	    for(int i=0;i<128;i++) {
	    	relativeNoteIndex = i - centerNote;

	    	octavesAway = abs(floor((float)relativeNoteIndex / (float)notesPerOctave));
	    	indexInOctave = relativeNoteIndex % notesPerOctave;
	    	octaveFactor = pow(octaveSize, octavesAway);

	    	if(relativeNoteIndex < 0) {
	    		octaveBaseFreq = centerFrequency / octaveFactor;
	    		indexInOctave = (indexInOctave + notesPerOctave) % notesPerOctave;
	    	} else {
	    		octaveBaseFreq = centerFrequency * octaveFactor;
	    	}

	    	noteRatio = ratios[indexInOctave];
	        pitchValues[i] = octaveBaseFreq * noteRatio;

			#ifdef DEBUG_TUNING
			Serial.print(i);
			Serial.print("\t");
			Serial.print(relativeNoteIndex);
			Serial.print("\t\t");
			Serial.print(octavesAway);
			Serial.print("\t\t");
			Serial.print(indexInOctave);
			Serial.print("\t\t");
			Serial.print(octaveBaseFreq);
			Serial.print("\t\t");
			Serial.print(noteRatio);
			Serial.print("\t\t");
			Serial.println(pitchValues[i]);
			#endif
	    }

	} else {
		// Create standard 12-tet note map
        for(int i=0;i<128;i++) {
            pitchValues[i] = getStandardFreq(i);
        }
	}

    return pitchValues;
}

float Tuning::getStandardFreq(float note) {
	return 440.0*(pow (1.059463094359,note - 69));
}
