#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "Dht22.hpp"

// FIXME: A shitty-written mockup of DHT22 sensor
#if 0
uint16_t receiveData(uint8_t dataPin_, uint8_t size) {
    uint16_t data = 0;
    unsigned long transmitDurations[size];
    unsigned long durations[size];

    for(int bitId = 0; bitId < size; bitId++) {
        // noInterrupts();

        unsigned long transmissionSignalStartTime = micros();

        // Wait for bit data
        while(!digitalRead(dataPin_))
            ;

        unsigned long bitStartTime = micros();
        //log("Got start of a bit.");

        while(digitalRead(dataPin_))
            ;

        unsigned long bitDuration = micros() - bitStartTime;
        unsigned long transmissionSignalDuration = bitStartTime - transmissionSignalStartTime;

        // interrupts();

        /*
        if(transmissionSignalDuration < 40 || transmissionSignalDuration > 60)
            log("Got an invalid transmission signal duration: ", transmissionSignalDuration);

        if(bitDuration < 20 || bitDuration > 30 && bitDuration < 70 || bitDuration > 80)
            log("Got an invalid bit duration: ", bitDuration);
        */

        durations[bitId] = bitDuration;
        if(bitDuration > 40)
            data |= (uint16_t(1) << (size - bitId - 1));
        // log("Bit value: ", bitDuration);
    }

    // for(int bitId = 0; bitId < size; bitId++)
        // log("Duration: ", transmitDurations[bitId], " ", durations[bitId]);

    return data;
}

int dataPin_ = DHT_22_SENSOR_PIN;
pinMode(dataPin_, OUTPUT);
digitalWrite(dataPin_, HIGH);

log("Waiting...");
delay(5000);

while(true) {
    log("Reading data...");

    // noInterrupts();

    digitalWrite(dataPin_, LOW);
    delay(10);

    digitalWrite(dataPin_, HIGH);
    delayMicroseconds(40);

    pinMode(dataPin_, INPUT);

    if(digitalRead(dataPin_)) {
        // interrupts();
        log("Can't get low voltage.");
        break;
    }
    unsigned long lowVoltageStartTime = micros();

    // Wait for high voltage
    while(!digitalRead(dataPin_))
        ;
    unsigned long highVoltageStartTime = micros();

    //log("Got high voltage!");

    // Wait transmission start
    while(digitalRead(dataPin_))
        ;

    unsigned long transmissionStartTime = micros();

    unsigned long lowVoltageDuration = highVoltageStartTime - lowVoltageStartTime;
    unsigned long highVoltageDuration = transmissionStartTime - highVoltageStartTime;

    // interrupts();

    if(lowVoltageDuration < 50 || lowVoltageDuration > 90)
        log("Got an invalid low voltage duration: ", lowVoltageDuration);
    if(highVoltageDuration < 60 || highVoltageDuration > 90)
        log("Got an invalid high voltage duration: ", highVoltageDuration);

    // uint16_t garbage = receiveData(dataPin_, 1);
    uint16_t first = receiveData(dataPin_, 16);
    uint16_t second = receiveData(dataPin_, 16);
    uint16_t checksum = receiveData(dataPin_, 8);

    uint8_t* data1 = reinterpret_cast<uint8_t*>(&first);
    uint8_t* data2 = reinterpret_cast<uint8_t*>(&second);
    uint8_t dataChecksum = *data1 + *(data1 + 1) + *data2 + *(data2 + 1);

    // interrupts();

    log("Result: ", first, " ", second, " ", checksum == dataChecksum);
    pinMode(dataPin_, OUTPUT);
    digitalWrite(dataPin_, HIGH);
    delay(3000);
}
#endif

using Util::Logging::log;
namespace Constants = Util::Constants;

typedef Dht22::Comfort Comfort;
enum class Dht22::Comfort: uint8_t {UNKNOWN, COLD, NORMAL, WARM, HOT};
static const char* COMFORT_NAMES[] = {"unknown", "cold", "normal", "warm", "hot"};

namespace {
    float getTemperature(uint16_t pwmValue) {
        float volts = float(pwmValue) / Constants::ANALOG_HIGH * Constants::VOLTS;
        return (volts - 0.5) * 100;
    }

    Comfort getComfort(float temperature) {
        long roundedTemperature = lround(temperature);

        if(roundedTemperature <= 18)
            return Comfort::COLD;
        else if(roundedTemperature <= 24)
            return Comfort::NORMAL;
        else if(roundedTemperature <= 26)
            return Comfort::WARM;
        else
            return Comfort::HOT;
    }    
}

Dht22::Dht22(uint8_t sensorPin, Util::TaskScheduler* scheduler, LedGroup* ledGroup, Buzzer* buzzer)
: sensorPin_(sensorPin), comfort_(Comfort::UNKNOWN), ledGroup_(ledGroup), ledProgress_(ledGroup), buzzer_(buzzer) {
    scheduler->addTask(&ledProgress_);
    scheduler->addTask(this);
}

void Dht22::execute() {
    uint16_t value = analogRead(sensorPin_);
    values_.add(value);

    if(values_.full()) {
        float temperature = getTemperature(value);
        float smoothedTemperature = getTemperature(values_.median());
        this->onTemperature(temperature, smoothedTemperature);
    }

    this->scheduleAfter(500);
}

void Dht22::onTemperature(float temperature, float smoothedTemperature) {
    Comfort comfort = getComfort(smoothedTemperature);

    if(comfort != comfort_)
        this->onComfortChange(comfort, comfort_ == Comfort::UNKNOWN);

    log("Temperature: ", temperature, " -> ", smoothedTemperature,
        " (", COMFORT_NAMES[int(comfort)], ").");
}

void Dht22::onComfortChange(Comfort comfort, bool initialChange) {
    comfort_ = comfort;
    ledGroup_->setLed(int(comfort));

    if(initialChange)
        ledProgress_.remove();
    else
        buzzer_->notify();
}
