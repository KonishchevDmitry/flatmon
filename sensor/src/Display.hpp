#ifndef Display_hpp
#define Display_hpp

#include <Util/Core.hpp>

#include <LiquidCrystal.h>

class Display {
    public:
        Display(uint8_t rsPin, uint8_t ePin, uint8_t d4Pin, uint8_t d5Pin, uint8_t d6Pin, uint8_t d7pin);

    public:

    private:
        LiquidCrystal lcd_;
};

#endif
