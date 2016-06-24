#ifndef Esp8266_hpp
#define Esp8266_hpp

#include <AltSoftSerial.h>

#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

class Esp8266: public Util::Task {
    private:
        enum class State: uint8_t;

    public:
        Esp8266(AltSoftSerial* serial, Util::TaskScheduler* scheduler);

    public:
        virtual void execute();

    private:
        AltSoftSerial* serial_;
        State state_;
};

#endif
