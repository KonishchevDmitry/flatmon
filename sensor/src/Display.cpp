#include <Util/Assertion.hpp>
#include <Util/Core.hpp>

#include "Display.hpp"

namespace {
    const int LCD_ROWS = 2;
    const int LCD_COLS = 16;


    const char TEMPERATURE_NO_DATA_TEXT[] = "??\xdf";
    const size_t TEMPERATURE_BLOCK_SIZE = sizeof TEMPERATURE_NO_DATA_TEXT - 1;

    const char HUMIDITY_NO_DATA_TEXT[] = " ??%";
    const size_t HUMIDITY_BLOCK_SIZE = sizeof HUMIDITY_NO_DATA_TEXT - 1;

    const char CO2_CONCENTRATION_NO_DATA_TEXT[] = " ???? ppm";
    const size_t CO2_CONCENTRATION_BLOCK_SIZE = sizeof CO2_CONCENTRATION_NO_DATA_TEXT - 1;

    const uint8_t CO2_CONCENTRATION_BLOCK_ROW = 0;
    const uint8_t CO2_CONCENTRATION_BLOCK_COL = LCD_COLS - CO2_CONCENTRATION_BLOCK_SIZE;

    const uint8_t HUMIDITY_BLOCK_ROW = CO2_CONCENTRATION_BLOCK_ROW;
    const uint8_t HUMIDITY_BLOCK_COL = CO2_CONCENTRATION_BLOCK_COL - HUMIDITY_BLOCK_SIZE;

    const uint8_t TEMPERATURE_BLOCK_ROW = HUMIDITY_BLOCK_ROW;
    const uint8_t TEMPERATURE_BLOCK_COL = HUMIDITY_BLOCK_COL - TEMPERATURE_BLOCK_SIZE;


    const char PRESSURE_NO_DATA_TEXT[] = "???\xfd?? mmHg";
    const size_t PRESSURE_BLOCK_SIZE = sizeof PRESSURE_NO_DATA_TEXT - 1;

    const uint8_t PRESSURE_BLOCK_ROW = 1;
    const uint8_t PRESSURE_BLOCK_COL = LCD_COLS - PRESSURE_BLOCK_SIZE;

}

Display::Display(uint8_t rsPin, uint8_t ePin, uint8_t d4Pin, uint8_t d5Pin, uint8_t d6Pin, uint8_t d7pin)
: lcd_(rsPin, ePin, d4Pin, d5Pin, d6Pin, d7pin), dataFlags_(0) {
    lcd_.begin(LCD_COLS, LCD_ROWS);
    this->resetTemperature(true);
    this->resetHumidity(true);
    this->resetCo2Concentration(true);
    this->resetPressure(true);
}

void Display::setTemperature(int8_t temperature) {
    if(this->hasData(DataType::temperature) && temperature_ == temperature)
        return;

    char buf[TEMPERATURE_BLOCK_SIZE + 1];
    size_t dataSize = snprintf(buf, sizeof buf, "%2hhd\xdf", temperature);
    if(dataSize != sizeof buf - 1)
        return this->resetTemperature();

    temperature_ = temperature;
    this->setData(DataType::temperature);
    this->setText(TEMPERATURE_BLOCK_ROW, TEMPERATURE_BLOCK_COL, buf);
}

void Display::resetTemperature(bool force) {
    if(force || this->hasData(DataType::temperature)) {
        this->resetData(DataType::temperature);
        this->setText(TEMPERATURE_BLOCK_ROW, TEMPERATURE_BLOCK_COL, TEMPERATURE_NO_DATA_TEXT);
    }
}

void Display::setHumidity(uint8_t humidity) {
    if(this->hasData(DataType::humidity) && humidity_ == humidity)
        return;

    char buf[HUMIDITY_BLOCK_SIZE + 1];
    size_t dataSize = snprintf(buf, sizeof buf, " %2hhu%%", humidity);
    if(dataSize != sizeof buf - 1)
        return this->resetHumidity();

    humidity_ = humidity;
    this->setData(DataType::humidity);
    this->setText(HUMIDITY_BLOCK_ROW, HUMIDITY_BLOCK_COL, buf);
}

void Display::resetHumidity(bool force) {
    if(force || this->hasData(DataType::humidity)) {
        this->resetData(DataType::humidity);
        this->setText(HUMIDITY_BLOCK_ROW, HUMIDITY_BLOCK_COL, HUMIDITY_NO_DATA_TEXT);
    }
}

void Display::setCo2Concentration(uint16_t co2Concentration) {
    if(this->hasData(DataType::co2Concentration) && co2Concentration_ == co2Concentration)
        return;

    char buf[CO2_CONCENTRATION_BLOCK_SIZE + 1];
    size_t dataSize = snprintf(buf, sizeof buf, " %4hu ppm", co2Concentration);
    if(dataSize != sizeof buf - 1)
        return this->resetCo2Concentration();

    co2Concentration_ = co2Concentration;
    this->setData(DataType::co2Concentration);
    this->setText(CO2_CONCENTRATION_BLOCK_ROW, CO2_CONCENTRATION_BLOCK_COL, buf);
}

void Display::resetCo2Concentration(bool force) {
    if(force || this->hasData(DataType::co2Concentration)) {
        this->resetData(DataType::co2Concentration);
        this->setText(CO2_CONCENTRATION_BLOCK_ROW, CO2_CONCENTRATION_BLOCK_COL, CO2_CONCENTRATION_NO_DATA_TEXT);
    }
}

void Display::setPressure(uint16_t pressure, uint8_t dispersion) {
    if(this->hasData(DataType::pressure) && pressure_ == pressure && pressureDispersion_ == dispersion)
        return;

    char buf[PRESSURE_BLOCK_SIZE + 1];
    size_t dataSize = snprintf(buf, sizeof buf, "%3hu\xfd%02hhu mmHg", pressure, dispersion);
    if(dataSize != sizeof buf - 1)
        return this->resetPressure();

    pressure_ = pressure;
    pressureDispersion_ = dispersion;
    this->setData(DataType::pressure);
    this->setText(PRESSURE_BLOCK_ROW, PRESSURE_BLOCK_COL, buf);
}

void Display::resetPressure(bool force) {
    if(force || this->hasData(DataType::pressure)) {
        this->resetData(DataType::pressure);
        this->setText(PRESSURE_BLOCK_ROW, PRESSURE_BLOCK_COL, PRESSURE_NO_DATA_TEXT);
    }
}

void Display::showAssertionError(
#if UTIL_VERBOSE_ASSERTS
    const FlashChar* file, int line
#endif
) {
    lcd_.clear();
    lcd_.print(F("Assertion error."));

#if UTIL_VERBOSE_ASSERTS
    if(LCD_ROWS < 2)
        return;

    lcd_.setCursor(0, 1);

    size_t lineLen = snprintf(nullptr, 0, "%d", line);
    if(lineLen >= LCD_COLS)
        return;

    const char* castedProgmemFile = reinterpret_cast<const char*>(file);

    size_t fileLen = strlen_P(castedProgmemFile);
    size_t allowedFileLen = LCD_COLS - 1 - lineLen;
    if(fileLen > allowedFileLen)
        castedProgmemFile += fileLen - allowedFileLen;

    lcd_.print(reinterpret_cast<const FlashChar*>(castedProgmemFile));
    lcd_.print(":");
    lcd_.print(line);
#endif
}

void Display::showSystemLockupError() {
    lcd_.clear();
    lcd_.print("System lockup");
    lcd_.setCursor(0, 1);
    lcd_.print("detected.");
}

bool Display::hasData(DataType dataType) {
    return dataFlags_ & uint8_t(dataType);
}

void Display::setData(DataType dataType) {
    dataFlags_ |= uint8_t(dataType);
}

void Display::resetData(DataType dataType) {
    dataFlags_ &= ~uint8_t(dataType);
}

void Display::setText(uint8_t row, uint8_t col, const char* text) {
    lcd_.setCursor(col, row);
    lcd_.print(text);
}
