/* 
 * MovingAverage.h - interface
 * MovingAverage, a simple implementation of a running average of 
 * number of values. 
 * Jouni Stenroos 2016
 */

#ifndef _RUNNINGAVERAGE_h_
#define _RUNNINGAVERAGE_h_

class MovingAverage {
public:
    MovingAverage(int nValues);
    int average(int value);
    int getAverage();
private:
    int *_values;
    int _nValues;
    int _curValue;
    bool _firstUse;
    int _total;
};

#endif // _RUNNINGAVERAGE_h_
