#include <RH_ASK.h>

#include <Util.h>
#include <Util/Assertion.hpp>
#include <Util/Logging.hpp>

#include <Common/SensorMessage.hpp>

using Util::Logging::log;
using Common::SensorMessage;

// FIXME: Add unusable PWM notes (because of Timer1 usage)
const int TRANSMITTER_SPEED = 2000;
const int TRANSMITTER_RX_PIN = 8;
const int TRANSMITTER_TX_PIN = A4;
const int TRANSMITTER_PTT_PIN = A5;
RH_ASK RECEIVER(TRANSMITTER_SPEED, TRANSMITTER_RX_PIN, TRANSMITTER_TX_PIN, TRANSMITTER_PTT_PIN);

void setup() {
    Util::Logging::init();
    Serial.begin(9600);

    log(F("Initializing..."));
    if(!RECEIVER.init())
        UTIL_LOGICAL_ERROR(F("Failed to initialize the receiver."));

    uint8_t buf[sizeof(SensorMessage) + 1];
    uint8_t messageSize;

    while(true) {
        messageSize = sizeof buf;
        if(!RECEIVER.recv(buf, &messageSize))
            continue;

        if(messageSize != sizeof(SensorMessage)) {
            log(F("Got a message with invalid size."));
            continue;
        }

        SensorMessage* message = reinterpret_cast<SensorMessage*>(buf);
        log(millis() / 1000, F(": Got a message from "), message->sensorId, F(" sensor: "), message->data, F("."));
    }
}

void loop() {
    UTIL_LOGICAL_ERROR();
}
