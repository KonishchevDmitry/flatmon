#include <Util/Assertion.hpp>
#include <Util/Core.hpp>

#include "Buzzer.hpp"

// If we configure buzzer pin as OUTPUT and set LOW level on it we hear a silent high frequency noise from it. Setting
// it as INPUT fixes the issue, so always switch it to INPUT when we aren't buzzing.

Buzzer::Buzzer(Util::TaskScheduler* scheduler, uint8_t pin)
: pin_(pin), state_(State::SILENT) {
    pinMode(pin, INPUT);
    scheduler->addTask(this);
    this->pause();
}

void Buzzer::notify() {
    if(state_ != State::SILENT)
        return;

    tone(pin_, 35);
    state_ = State::BUZZING_1;
    this->scheduleAfter(100);
    this->resume();
}

void Buzzer::execute() {
    switch(state_) {
        case State::BUZZING_1:
            tone(pin_, 41, 50);
            state_ = State::BUZZING_2;
            this->scheduleAfter(50);
            break;
        case State::BUZZING_2:
            noTone(pin_);
            pinMode(pin_, INPUT);
            state_ = State::TIMEOUT;
            this->scheduleAfter(1000);
            break;
        case State::TIMEOUT:
            state_ = State::SILENT;
            this->pause();
            break;
        default:
            UTIL_ASSERT(false);
            break;
    }
}
