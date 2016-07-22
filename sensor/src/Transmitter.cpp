#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include <Common/SensorMessage.hpp>

#include "Config.hpp"
#include "Transmitter.hpp"

using Util::Logging::vlog;
using Common::SensorMessage;
namespace Constants = Util::Constants;

Transmitter::Transmitter(RH_ASK* transmitter, Util::TaskScheduler* scheduler,
                         const Dht22* dht22, const CO2Sensor* co2Sensor)
: transmitter_(transmitter), dht22_(dht22), co2Sensor_(co2Sensor) {
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

    uint16_t co2Concentration = SensorMessage::UNKNOWN_CO2_CONCENTRATION;
    if(co2Sensor_)
        co2Sensor_->getConcentration(&co2Concentration);

    SensorMessage message = {
        sensorId: Config::SENSOR_ID,
        temperature: temperature,
        humidity: humidity,
        co2Concentration: co2Concentration,
    };

    transmitter_->send(reinterpret_cast<const uint8_t*>(&message), sizeof message);
    this->scheduleAfter(Constants::SECOND_MILLIS);
}
