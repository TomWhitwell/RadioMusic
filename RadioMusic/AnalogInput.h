#ifndef AnalogInput_h
#define AnalogInput_h

#include "Arduino.h"

#define ADC_BITS 13
#define ADC_MAX_VALUE (1 << ADC_BITS)

class AnalogInput {
	public:
		AnalogInput(uint pinIndex);
		boolean update();
		void setRange(float outLow, float outHigh, boolean quantise);
		void setAverage(boolean avg);
		void setSmoothSteps(int steps);

		void printDebug();
		float getRatio();
		float currentValue = -100.0;
		int32_t inputValue = 0;
		uint16_t borderThreshold = 16;
	private:
		int pin;
		float outputLow = 0.0;
		float outputHigh = 1.0;
		float inToOutRatio = 0.0;
		float inverseRatio = 0.0;

		// Set out of range to trigger a change status on first call.
		int32_t oldInputValue = -1000;
		int32_t valueAtLastChange = -1000;

		// Use hysteresis thresholds at value boundaries
		boolean hysteresis = false;

		// Clamp output to int
		boolean quantise = false;

		// Smooth input
		boolean average = false;
		uint8_t smoothSteps = 40;
};

#endif
