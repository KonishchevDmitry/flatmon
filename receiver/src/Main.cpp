#include <RH_ASK.h>

#include <Util.h>
#include <Util/Assertion.hpp>
#include <Util/Constants.hpp>
#include <Util/Logging.hpp>

#include <Common/SensorMessage.hpp>

namespace Constants = Util::Constants;

using Util::Logging::log_error;
using Util::Logging::log_warning;
using Util::Logging::log_info;

using Common::SensorMessage;

namespace {

// RadioHead breaks PWM on the following pins because of timer usage:
//
//    Board       Timer  Unusable PWM
// Arduino Uno   Timer1         9, 10
// Arduino Mega  Timer1        11, 12
const int TRANSMITTER_SPEED = 1000;
const int TRANSMITTER_RX_PIN = 8;
RH_ASK_RECEIVER RECEIVER(TRANSMITTER_SPEED, TRANSMITTER_RX_PIN);

}

void setup() {
    Util::Core::init();
    Util::Logging::init();
    if(!RECEIVER.init())
        UTIL_LOGICAL_ERROR(F("Failed to initialize the receiver."));

    log_info(F("Listening to messages from sensors..."));

    TimeMillis lastStatsTime = millis();
    TimeMillis lastHeartbeatTime = millis();

    uint8_t sensorMessageBuf[sizeof(SensorMessage) + 1];
    uint8_t messageSize;

    while(true) {
        messageSize = sizeof sensorMessageBuf;

        if(!RECEIVER.recv(sensorMessageBuf, &messageSize)) {
            TimeMillis curTime = millis();
            bool noMessagesTimeoutExceeded = curTime - lastHeartbeatTime >= Constants::MINUTE_MILLIS;

            if(noMessagesTimeoutExceeded) {
                log_warning(F("I'm alive but there are no messages from sensors."));
                lastHeartbeatTime = curTime;
            }

            if(noMessagesTimeoutExceeded || curTime - lastStatsTime >= Constants::MINUTE_MILLIS) {
                log_info(F("Received messages statistics: "),
                         RECEIVER.rxGood(), F(" good, "), RECEIVER.rxBad(), F(" bad."));
                lastStatsTime = curTime;
            }

            continue;
        }

        lastHeartbeatTime = millis();

        if(messageSize != sizeof(SensorMessage)) {
            log_error(F("Got a message with invalid size."));
            continue;
        }

        SensorMessage* message = reinterpret_cast<SensorMessage*>(sensorMessageBuf);
        if(message->messageType != SensorMessage::MESSAGE_TYPE) {
            log_error(F("Got a message with invalid message type."));
            continue;
        }

        log_info(F("Got a message from #"), message->sensorId, F(" sensor:"));
        uint16_t checksum = uint16_t(message->sensorId) + message->temperature + message->humidity +
                            message->co2Concentration + message->pressure;
        log_info(F("> "), message->sensorId, ",", message->temperature, ",", message->humidity, ",",
            message->co2Concentration, ",", message->pressure, ",", checksum);
    }
}

void loop() {
    UTIL_LOGICAL_ERROR();
}
