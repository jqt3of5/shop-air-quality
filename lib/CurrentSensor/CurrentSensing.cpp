//
// Created by jqt3o on 10/14/2022.
//

#include <esp32-hal-adc.h>
#include "CurrentSensing.h"

CurrentSensing::CurrentSensing(int pin, int refmV, int mvPerAmp) : _pin(pin), _index(0) {

    _historyTotal = 0 ;
    _adcPerAmp = (float)MAX_ADC/refmV * mvPerAmp;
}

void CurrentSensing::begin() {
    adcAttachPin(_pin);

    pinMode(_pin, INPUT);
    _historyTotal = 0;
    for(int i = 0; i < _max; ++i)
    {
        _y[i] = 0;
        _history[i] = 0;
    }
}

double CurrentSensing::measure(int samples) {

    double avgRms = 0;
    //TODO: Would be better to measure on crossings, not sample count

    _history[0] = analogRead(_pin);
    _historyTotal = _history[0];
    _y[0] = 0;
    _index = 0;

    for (int i = 0; i < _max*2; ++i)
    {
        auto sample = analogRead(_pin);

        //_y is filled with zeroes, so the first pass this will be a noop
        _historyTotal -= _y[_index];

        //high pass filter
        _y[_index] = _y[_index] + sample - _history[_index];
        _history[_index] = sample;
        _historyTotal += _y[_index];

        _index = (_index + 1) % _max;

        if (i >= _max)
        {
            auto low = (double)_historyTotal / _max;
            avgRms += low * low;
        }

        delayMicroseconds(250);
    }

    return sqrt(avgRms/_max) / _adcPerAmp;
}