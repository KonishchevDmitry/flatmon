#ifndef Buzzer_hpp
#define Buzzer_hpp

#include <Arduino.h>

#include <Util/TaskScheduler.hpp>

// Attention: Use of tone() function interferes with PWM output on pins 3 and 11 (on boards other than the Mega).
class Buzzer: public Util::Task {
    private:
        enum class State: uint8_t {SILENT, BUZZING_1, BUZZING_2, TIMEOUT};

    public:
        Buzzer(Util::TaskScheduler* scheduler, uint8_t pin);

    public:
        virtual void execute();
        void notify();

    private:
        uint8_t pin_;
        State state_;
};

#endif

