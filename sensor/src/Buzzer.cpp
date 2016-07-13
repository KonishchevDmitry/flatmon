#include <Util/Assertion.hpp>
#include <Util/Core.hpp>

#include "Buzzer.hpp"

Buzzer::Buzzer(Util::TaskScheduler* scheduler, uint8_t pin)
: pin_(pin), state_(State::SILENT) {
    pinMode(pin, OUTPUT);
    scheduler->addTask(this);
    this->pause();
}

void Buzzer::notify() {
    if(state_ != State::SILENT)
        return;

    tone(pin_, 35);
    this->scheduleAfter(100);
    state_ = State::BUZZING_1;

    this->resume();
}

void Buzzer::execute() {
    switch(state_) {
        case State::BUZZING_1:
            tone(pin_, 41, 50);
            this->scheduleAfter(50);
            state_ = State::BUZZING_2;
            break;
        case State::BUZZING_2:
            this->scheduleAfter(1000);
            state_ = State::TIMEOUT;
            break;
        case State::TIMEOUT:
            this->pause();
            state_ = State::SILENT;
            break;
        default:
            UTIL_ASSERT(false);
            break;
    }
}
