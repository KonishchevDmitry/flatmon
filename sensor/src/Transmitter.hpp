#ifndef Transmitter_hpp
#define Transmitter_hpp

#include <RH_ASK.h>

#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

#include "Dht22.hpp"

class Transmitter: public Util::Task {
    public:
        Transmitter(RH_ASK* transmitter, Util::TaskScheduler* scheduler, Dht22* dht22);

    public:
        virtual void execute();

    private:
        RH_ASK* transmitter_;
        Dht22* dht22_;
};

#endif
