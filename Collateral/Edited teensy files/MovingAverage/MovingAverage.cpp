/*
 * MovingAverage.cpp - implementation
 * Jouni Stenroos 2016
 */

#include <math.h>
#include "MovingAverage.h"

MovingAverage::MovingAverage(int nValues) {
    if (nValues <= 1) {
	_values = NULL;
	return;
    }
    _values = new int[nValues];
    _nValues = nValues;
    for (int i = 0; i < nValues; i++) {
	_values[i] = 0;
    }
    _firstUse = true;
    _curValue = 0;
    _total = 0;
}

int MovingAverage::average(int aValue) {
    if (_values == NULL)
	return 0;
    if (_firstUse) {
	/* First time fill all values with the value */
	for (int i = 0; i < _nValues; i++) {
	    _values[i] = aValue;
	    _total += aValue;
	}
	_firstUse = false;
	_curValue++;
    } else {
	_total -= _values[_curValue];
	_total += aValue;
	_values[_curValue++] = aValue;
	if (_curValue >= _nValues)
	    _curValue = 0;
    }
    return (int)round((double)_total / (double)_nValues);
}

int MovingAverage::getAverage() {
    return (int)round((double)_total / (double)_nValues);
}
