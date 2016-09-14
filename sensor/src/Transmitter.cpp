#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include <Common/SensorMessage.hpp>

#include "Config.hpp"
#include "Transmitter.hpp"

using Util::Logging::log_debug;
using Common::SensorMessage;
namespace Constants = Util::Constants;

Transmitter::Transmitter(RH_ASK* transmitter, Util::TaskScheduler* scheduler,
                         const Dht22* dht22, const Co2Sensor* co2Sensor, PressureSensor* pressureSensor)
: transmitter_(transmitter), dht22_(dht22), co2Sensor_(co2Sensor), pressureSensor_(pressureSensor) {
    if(!transmitter->init())
        UTIL_LOGICAL_ERROR(F("Failed to initialize the transmitter."));

    scheduler->addTask(this);
}

void Transmitter::execute() {
    log_debug(F("Sending sensor data..."));

    uint8_t temperature = 0;
    int8_t signedTemperature;
    if(dht22_->getTemperature(&signedTemperature) && signedTemperature > 0)
        temperature = signedTemperature;

    uint8_t humidity = 0;
    dht22_->getHumidity(&humidity);

    uint16_t co2Concentration = 0;
    co2Sensor_->getConcentration(&co2Concentration);

    uint16_t pressure = 0;
    pressureSensor_->getPressure(&pressure);

    SensorMessage message = {
        messageType: SensorMessage::MESSAGE_TYPE,
        sensorId: Config::SENSOR_ID,
        temperature: temperature,
        humidity: humidity,
        co2Concentration: co2Concentration,
        pressure: pressure,
    };

    if(message.temperature != temperature)
        message.temperature = 0;
    if(message.humidity != humidity)
        message.humidity = 0;
    if(message.co2Concentration != co2Concentration)
        message.co2Concentration = 0;
    if(message.pressure != pressure)
        message.pressure = 0;

    transmitter_->send(reinterpret_cast<const uint8_t*>(&message), sizeof message);
    this->scheduleAfter(10 * Constants::SECOND_MILLIS);
}
