#ifndef Display_hpp
#define Display_hpp

#include <Util/Core.hpp>

#include <LiquidCrystal.h>

class Display {
    private:
        typedef int16_t Data;

    public:
        Display(uint8_t rsPin, uint8_t ePin, uint8_t d4Pin, uint8_t d5Pin, uint8_t d6Pin, uint8_t d7pin);

    public:
        void setTemperature(int8_t temperature);
        void resetTemperature();

        void setHumidity(uint8_t humidity);
        void resetHumidity();

        void setCo2Concentration(uint16_t concentration);
        void resetCo2Concentration();

        void showAssertionError(
        #if UTIL_VERBOSE_ASSERTS
            const FlashChar* file, int line
        #endif
        );

        void showSystemLockupError();

    private:
        void setText(uint8_t row, uint8_t col, const char* text);

    private:
        LiquidCrystal lcd_;

        Data temperature_;
        Data humidity_;
        Data co2Concentration_;

        static constexpr Data NO_DATA_ = 1 << 15;
};

#endif
