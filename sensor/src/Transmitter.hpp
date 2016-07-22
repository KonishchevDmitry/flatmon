#ifndef Transmitter_hpp
#define Transmitter_hpp

#include <RH_ASK.h>

#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

#include "CO2Sensor.hpp"
#include "Dht22.hpp"

class Transmitter: public Util::Task {
    public:
        Transmitter(RH_ASK* transmitter, Util::TaskScheduler* scheduler,
                    const Dht22* dht22, const CO2Sensor* co2Sensor = nullptr);

    public:
        virtual void execute();

    private:
        RH_ASK* transmitter_;
        const Dht22* dht22_;
        const CO2Sensor* co2Sensor_;
};

#endif
