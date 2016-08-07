#include <Util/Assertion.hpp>
#include <Util/Core.hpp>

#include "Display.hpp"

namespace {
    const int LCD_ROWS = 2;
    const int LCD_COLS = 16;

    const char TEMPERATURE_NO_DATA_TEXT[] = " ??\xdf";
    const size_t TEMPERATURE_BLOCK_SIZE = sizeof TEMPERATURE_NO_DATA_TEXT - 1;

    const char HUMIDITY_NO_DATA_TEXT[] = " ??%";
    const size_t HUMIDITY_BLOCK_SIZE = sizeof TEMPERATURE_NO_DATA_TEXT - 1;

    const char CO2_CONCENTRATION_NO_DATA_TEXT[] = "???? ppm";
    const size_t CO2_CONCENTRATION_BLOCK_SIZE = sizeof CO2_CONCENTRATION_NO_DATA_TEXT - 1;

    const uint8_t HUMIDITY_BLOCK_ROW = 0;
    const uint8_t HUMIDITY_BLOCK_COL = LCD_COLS - HUMIDITY_BLOCK_SIZE;

    const uint8_t TEMPERATURE_BLOCK_ROW = HUMIDITY_BLOCK_ROW;
    const uint8_t TEMPERATURE_BLOCK_COL = HUMIDITY_BLOCK_COL - TEMPERATURE_BLOCK_SIZE;

    const uint8_t CO2_CONCENTRATION_BLOCK_ROW = 1;
    const uint8_t CO2_CONCENTRATION_BLOCK_COL = LCD_COLS - CO2_CONCENTRATION_BLOCK_SIZE;
}

Display::Display(uint8_t rsPin, uint8_t ePin, uint8_t d4Pin, uint8_t d5Pin, uint8_t d6Pin, uint8_t d7pin)
: lcd_(rsPin, ePin, d4Pin, d5Pin, d6Pin, d7pin), temperature_(0), humidity_(0), co2Concentration_(0) {
    lcd_.begin(LCD_COLS, LCD_ROWS);
    this->resetTemperature();
    this->resetHumidity();
    this->resetCo2Concentration();
}

void Display::setTemperature(int8_t temperature) {
    if(temperature_ == temperature)
        return;

    char buf[TEMPERATURE_BLOCK_SIZE + 1];
    size_t dataSize = snprintf(buf, sizeof buf, "% 2hd\xdf", temperature);
    UTIL_ASSERT(dataSize == sizeof buf - 1);

    this->setText(TEMPERATURE_BLOCK_ROW, TEMPERATURE_BLOCK_COL, buf);
    temperature_ = temperature;
}

void Display::resetTemperature() {
    if(temperature_ == NO_DATA_)
        return;

    this->setText(TEMPERATURE_BLOCK_ROW, TEMPERATURE_BLOCK_COL, TEMPERATURE_NO_DATA_TEXT);
    temperature_ = NO_DATA_;
}

void Display::setHumidity(uint8_t humidity) {
    if(humidity_ == humidity)
        return;

    char buf[HUMIDITY_BLOCK_SIZE + 1];
    size_t dataSize = snprintf(buf, sizeof buf, " %2hd%%", humidity);
    UTIL_ASSERT(dataSize == sizeof buf - 1);

    this->setText(HUMIDITY_BLOCK_ROW, HUMIDITY_BLOCK_COL, buf);
    humidity_ = humidity;
}

void Display::resetHumidity() {
    if(humidity_ == NO_DATA_)
        return;

    this->setText(HUMIDITY_BLOCK_ROW, HUMIDITY_BLOCK_COL, HUMIDITY_NO_DATA_TEXT);
    humidity_ = NO_DATA_;
}

void Display::setCo2Concentration(uint16_t concentration) {
    Data co2Concentration = concentration;
    UTIL_ASSERT(co2Concentration >= 0); // Check uint16_t -> Data (int16_t) conversion

    if(co2Concentration_ == co2Concentration)
        return;

    char buf[CO2_CONCENTRATION_BLOCK_SIZE + 1];
    size_t dataSize = snprintf(buf, sizeof buf, "%4hd ppm", co2Concentration);
    UTIL_ASSERT(dataSize == sizeof buf - 1);

    this->setText(CO2_CONCENTRATION_BLOCK_ROW, CO2_CONCENTRATION_BLOCK_COL, buf);
    co2Concentration_ = co2Concentration;
}

void Display::resetCo2Concentration() {
    if(co2Concentration_ == NO_DATA_)
        return;

    this->setText(CO2_CONCENTRATION_BLOCK_ROW, CO2_CONCENTRATION_BLOCK_COL, CO2_CONCENTRATION_NO_DATA_TEXT);
    co2Concentration_ = NO_DATA_;
}

void Display::setText(uint8_t row, uint8_t col, const char* text) {
    lcd_.setCursor(col, row);
    lcd_.print(text);
}
