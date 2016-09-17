#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include <Common/SensorMessage.hpp>

#include "Config.hpp"
#include "Transmitter.hpp"

using Util::Logging::log_debug;
using Common::SensorMessage;
namespace Constants = Util::Constants;

namespace {
    const TimeMillis SENDING_PERIOD = 10 * Constants::SECOND_MILLIS;
    const uint8_t PRESCALER_MASK = 1 << CS12 || 1 << CS11 || 1 << CS10;
}

enum class Transmitter::State: uint8_t {send, sending};
Transmitter::StateHandler Transmitter::stateHandlers_[] = {
    &Transmitter::onSend,
    &Transmitter::onSending,
};

Transmitter::Transmitter(RH_ASK* transmitter, Util::TaskScheduler* scheduler,
                         const Dht22* dht22, const Co2Sensor* co2Sensor, PressureSensor* pressureSensor)
: transmitter_(transmitter), dht22_(dht22), co2Sensor_(co2Sensor), pressureSensor_(pressureSensor),
  state_(State::send) {
    if(!transmitter->init())
        UTIL_LOGICAL_ERROR(F("Failed to initialize the transmitter."));

    // RH_ASK doesn't stop timer when it's not needed and the running timer affects time-sensitive tasks, so we stop and
    // start it manually.
    prescaler_ = TCCR1B & PRESCALER_MASK;
    this->stopTimer();

    scheduler->addTask(this);
    this->scheduleAfter(SENDING_PERIOD);
}

void Transmitter::execute() {
    size_t handlerId = size_t(state_);
    UTIL_ASSERT(handlerId < UTIL_ARRAY_SIZE(stateHandlers_));
    (this->*stateHandlers_[handlerId])();
}

void Transmitter::onSend() {
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

    this->startTimer();
    transmitter_->send(reinterpret_cast<const uint8_t*>(&message), sizeof message);

    // Each message is transmitted as:
    //
    // - 36 bit training preamble consisting of 0-1 bit pairs
    // - 12 bit start symbol 0xb38
    // - 1 byte of message length byte count (4 to 30), count includes byte count and FCS bytes
    // - n message bytes (uincluding 4 bytes of header), maximum n is RH_ASK_MAX_MESSAGE_LEN + 4 (64)
    // - 2 bytes FCS, sent low byte-hi byte
    //
    // Everything after the start symbol is encoded 4 to 6 bits.
    size_t messageSizeBits = 36 + 12 + (1 + 4 + sizeof message + 2) * 12;

    state_ = State::sending;
    this->scheduleAfter(messageSizeBits * Constants::SECOND_MILLIS / transmitter_->speed());
}

void Transmitter::onSending() {
    if(transmitter_->mode() != RH_ASK::RHModeIdle)
        return;

    this->stopTimer();

    state_ = State::send;
    this->scheduleAfter(SENDING_PERIOD);
}

void Transmitter::stopTimer() {
    TCCR1B &= ~PRESCALER_MASK;
}

void Transmitter::startTimer() {
    TCCR1B |= prescaler_;
}
