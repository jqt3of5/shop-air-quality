//
// Created by jqt3o on 10/14/2022.
//

#ifndef SHOP_AIR_QUALITY_CURRENTSENSING_H
#define SHOP_AIR_QUALITY_CURRENTSENSING_H

#define MAX_ADC 4096
class CurrentSensing {
public:
    explicit CurrentSensing(int pin, int refmV = 2450, int mvPerAmp = 50);
    void begin();

    double measure(int samples = 1000);

private:
    int _pin;
    float _adcPerAmp;

    int _historyTotal;
    const static uint8_t _max = 32;
    uint16_t _history[_max];
    int _y[_max];
    uint8_t _index;
};


#endif //SHOP_AIR_QUALITY_CURRENTSENSING_H
