#include <Util/Core.hpp>

#include "Display.hpp"

Display::Display(uint8_t rsPin, uint8_t ePin, uint8_t d4Pin, uint8_t d5Pin, uint8_t d6Pin, uint8_t d7pin)
: lcd_(rsPin, ePin, d4Pin, d5Pin, d6Pin, d7pin) {
    lcd_.begin(16, 2);
    lcd_.print(" 27\xdf");
    lcd_.print(" 43%");
    lcd_.setCursor(0, 1);
    lcd_.print("1543 ppm");
}
