#include <Util/Assertion.hpp>
#include <Util/Core.hpp>

#include "Indication.hpp"

ShiftRegisterLeds::ShiftRegisterLeds(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin)
: dataPin_(dataPin), clockPin_(clockPin), latchPin_(latchPin), value_(0) {
    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(latchPin, OUTPUT);
    this->set(value_);
}

void ShiftRegisterLeds::set(LedsValue leds) {
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        int curByte = sizeof leds - 1, lastByte = 0, incByte = -1;
    #else
        int curByte = 0, lastByte = sizeof leds - 1, incByte = 1;
    #endif
#else
    #error Unable to determine platform endianness
#endif

    auto bytes = reinterpret_cast<const byte*>(&leds);

    while(true) {
        shiftOut(dataPin_, clockPin_, MSBFIRST, bytes[curByte]);
        if(curByte == lastByte)
            break;
        curByte += incByte;
    }

    digitalWrite(latchPin_, HIGH);
    digitalWrite(latchPin_, LOW);

    value_ = leds;
}

void ShiftRegisterLeds::update(LedsValue leds, LedsValue mask) {
    auto valueWithoutGroup = value_ & ~mask;
    this->set(valueWithoutGroup | leds);
}


LedGroup::LedGroup(ShiftRegisterLeds* leds, uint8_t startBit, uint8_t ledsNum)
: ledsNum(ledsNum), leds_(leds), startBit_(startBit), mask_(0) {
    UTIL_ASSERT(startBit_ + ledsNum <= ShiftRegisterLeds::MAX_LEDS_NUM);
    for(uint8_t ledId = 0; ledId < ledsNum; ledId++)
        mask_ |= LedsValue(1) << (startBit_ + ledId);
}

void LedGroup::setLed(uint8_t ledNum) {
    UTIL_ASSERT(ledNum <= this->ledsNum);

    LedsValue value = ledNum
        ? LedsValue(1) << (startBit_ + ledNum - 1)
        : 0;

    leds_->update(value, mask_);
}


LedProgressTask::LedProgressTask(LedGroup* ledGroup)
: ledGroup_(ledGroup), curLedNum_(0) {
}

void LedProgressTask::execute() {
    curLedNum_ = curLedNum_ >= ledGroup_->ledsNum ? 0 : curLedNum_ + 1;
    ledGroup_->setLed(curLedNum_);
    this->scheduleAfter(100);
}
