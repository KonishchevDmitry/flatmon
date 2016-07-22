#ifndef Common_SensorMessage_hpp
#define Common_SensorMessage_hpp

#include <Util/Core.hpp>

namespace Common {

// FIXME: add some message type for validation
// FIXME: pack into bits instead of bytes
struct SensorMessage {
    // FIXME: Alter the values
    static constexpr uint8_t UNKNOWN_HUMIDITY = 127;
    static constexpr int8_t UNKNOWN_TEMPERATURE = 127;
    static constexpr uint16_t UNKNOWN_CO2_CONCENTRATION = 0xFFFF;

    uint8_t sensorId;
    int8_t temperature;
    uint8_t humidity;
    uint16_t co2Concentration;
} __attribute__((packed));

}

#endif
