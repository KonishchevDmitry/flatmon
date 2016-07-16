#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include <Common/SensorMessage.hpp>

#include "Config.hpp"
#include "Transmitter.hpp"

using Util::Logging::vlog;
using Common::SensorMessage;
namespace Constants = Util::Constants;

Transmitter::Transmitter(RH_ASK* transmitter, Util::TaskScheduler* scheduler, Dht22* dht22)
: transmitter_(transmitter), dht22_(dht22) {
    if(!transmitter->init())
        UTIL_LOGICAL_ERROR(F("Failed to initialize the transmitter."));

    scheduler->addTask(this);
}

void Transmitter::execute() {
    log("Sending sensor data...");

    uint8_t humidity = SensorMessage::UNKNOWN_HUMIDITY;
    dht22_->getHumidity(&humidity);

    int8_t temperature = SensorMessage::UNKNOWN_TEMPERATURE;
    dht22_->getTemperature(&temperature);

    SensorMessage message = {
        sensorId: Config::SENSOR_ID,
        humidity: humidity,
        temperature: temperature,
    };

    transmitter_->send(reinterpret_cast<const uint8_t*>(&message), sizeof message);
    this->scheduleAfter(Constants::SECOND_MILLIS);
}
