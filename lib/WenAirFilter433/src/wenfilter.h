//
// Created by jqt3o on 10/9/2022.
//

#ifndef SHOP_AIR_QUALITY_RADIO_H
#define SHOP_AIR_QUALITY_RADIO_H

#include <stdint.h>

//The values indicate the number of times to repeat the command to set this value
enum WenFilterSpeed {
    Low = 1,
    Medium = 2,
    High = 3,
};

//The values indicate the number of times to repeat the command to set this value
enum WenFilterTime {
    None = 0,
    OneHour = 1,
    TwoHour = 2,
    FourHour = 3
};

enum WenCommand{
    Speed = 0b1100001110011110,
    Time = 0b1100001100110111,
    Off= 0b1100001110010001
};

class WenFilter {
public:
   WenFilter(int pin);

   void begin();

   void setOffClear();
   void setOnState(WenFilterSpeed speed, WenFilterTime time);
   void writeCommand(WenCommand command);

private:
    void writeBlock(int16_t bits) const;
    void writePreamble() const;
    int _pin;
};
#endif //SHOP_AIR_QUALITY_RADIO_H
