#ifndef Display_hpp
#define Display_hpp

#include <Util/Core.hpp>

#include <LiquidCrystal.h>

class Display {
    private:
        enum class DataType: uint8_t {
            temperature      = 1 << 0,
            humidity         = 1 << 1,
            co2Concentration = 1 << 2,
            pressure         = 1 << 3,
        };

    public:
        Display(uint8_t rsPin, uint8_t ePin, uint8_t d4Pin, uint8_t d5Pin, uint8_t d6Pin, uint8_t d7pin);

    public:
        void setTemperature(int8_t temperature);
        void resetTemperature(bool force = false);

        void setHumidity(uint8_t humidity);
        void resetHumidity(bool force = false);

        void setCo2Concentration(uint16_t co2Concentration);
        void resetCo2Concentration(bool force = false);

        void setPressure(uint16_t pressure, uint8_t dispersion);
        void resetPressure(bool force = false);

        void showAssertionError(
        #if UTIL_VERBOSE_ASSERTS
            const FlashChar* file, int line
        #endif
        );

        void showSystemLockupError();

    private:
        bool hasData(DataType dataType);
        void setData(DataType dataType);
        void resetData(DataType dataType);
        void setText(uint8_t row, uint8_t col, const char* text);

    private:
        LiquidCrystal lcd_;

        uint8_t dataFlags_;
        int8_t temperature_;
        uint8_t humidity_;
        uint16_t co2Concentration_;
        uint16_t pressure_;
        uint8_t pressureDispersion_;
};

#endif
