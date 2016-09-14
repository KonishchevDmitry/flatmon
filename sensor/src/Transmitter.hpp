#ifndef Transmitter_hpp
#define Transmitter_hpp

#include <RH_ASK.h>

#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

#include "Co2Sensor.hpp"
#include "Dht22.hpp"
#include "PressureSensor.hpp"

class Transmitter: public Util::Task {
    public:
        Transmitter(RH_ASK* transmitter, Util::TaskScheduler* scheduler,
                    const Dht22* dht22, const Co2Sensor* co2Sensor, PressureSensor* pressureSensor);

    public:
        virtual const FlashChar* getName() { return F("Transmitter"); }
        virtual void execute();

    private:
        RH_ASK* transmitter_;
        const Dht22* dht22_;
        const Co2Sensor* co2Sensor_;
        const PressureSensor* pressureSensor_;
};

#endif
