#ifndef Common_SensorMessage_hpp
#define Common_SensorMessage_hpp

#include <Util/Core.hpp>

namespace Common {

// FIXME: add some message type for validation
// FIXME: pack into bits instead of bytes
struct SensorMessage {
    uint8_t sensorId;
    uint8_t data;
} __attribute__((packed));

}

#endif
