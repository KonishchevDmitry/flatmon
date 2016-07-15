#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include <Common/SensorMessage.hpp>

#include "Transmitter.hpp"

using Util::Logging::vlog;
namespace Constants = Util::Constants;

Transmitter::Transmitter(RH_ASK* transmitter, Util::TaskScheduler* scheduler)
: transmitter_(transmitter) {
    if(!transmitter->init())
        UTIL_LOGICAL_ERROR(F("Failed to initialize the transmitter."));

    scheduler->addTask(this);
}

void Transmitter::execute() {
    vlog("Sending sensor data...");
    Common::SensorMessage message = {sensorId: 1, data: 42};
    transmitter_->send(reinterpret_cast<const uint8_t*>(&message), sizeof message);
    this->scheduleAfter(Constants::SECOND_MILLIS);
}
