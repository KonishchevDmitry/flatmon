#include <RH_ASK.h>

#include <Util.h>
#include <Util/Assertion.hpp>
#include <Util/Logging.hpp>

#include <Common/SensorMessage.hpp>

using Util::Logging::log;
using Common::SensorMessage;

namespace {

// FIXME: Add unusable PWM notes (because of Timer1 usage)
const int TRANSMITTER_SPEED = 2000;
const int TRANSMITTER_RX_PIN = 8;
const int TRANSMITTER_TX_PIN = A4;
const int TRANSMITTER_PTT_PIN = A5;
RH_ASK RECEIVER(TRANSMITTER_SPEED, TRANSMITTER_RX_PIN, TRANSMITTER_TX_PIN, TRANSMITTER_PTT_PIN);

char bitsToHex(uint8_t bits) {
    UTIL_ASSERT(bits <= 15);
    return bits < 10 ? '0' + bits : 'A' + bits - 10;
}

// FIXME: add checksum with invertion (~)
void sendMessageToServer(const void* data, size_t size) {
    char buf[size * 2 + 1];
    size_t bufPos = 0;

    for(size_t byteId = 0; byteId < size; byteId++) {
        uint8_t value = reinterpret_cast<const uint8_t*>(data)[byteId];
        buf[bufPos++] = bitsToHex(value >> 4);
        buf[bufPos++] = bitsToHex(value & 0b1111);
    }
    buf[bufPos] = '\0';

    log(F("> "), buf);
}

}

void setup() {
    Util::Logging::init();
    Serial.begin(9600);

    log(F("Initializing..."));
    if(!RECEIVER.init())
        UTIL_LOGICAL_ERROR(F("Failed to initialize the receiver."));

    uint8_t sensorMessageBuf[sizeof(SensorMessage) + 1];
    uint8_t messageSize;

    while(true) {
        messageSize = sizeof sensorMessageBuf;
        if(!RECEIVER.recv(sensorMessageBuf, &messageSize))
            continue;

        if(messageSize != sizeof(SensorMessage)) {
            log(F("Got a message with invalid size."));
            continue;
        }

        SensorMessage* message = reinterpret_cast<SensorMessage*>(sensorMessageBuf);
        log(millis() / 1000, F(": Got a message from #"), message->sensorId, F(" sensor."));
        sendMessageToServer(message, sizeof(SensorMessage));
    }
}

void loop() {
    UTIL_LOGICAL_ERROR();
}
