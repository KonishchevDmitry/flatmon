#ifndef Common_SensorMessage_hpp
#define Common_SensorMessage_hpp

#include <Util/Core.hpp>

namespace Common {

struct SensorMessage {
    static constexpr uint16_t MESSAGE_TYPE = 0b1001001;
    uint16_t messageType      :  7;
    uint8_t  sensorId         :  3;
    uint8_t  temperature      :  6;
    uint8_t  humidity         :  7;
    uint16_t co2Concentration : 12;
    uint16_t pressure         : 13;
} __attribute__((packed));

}

#endif
