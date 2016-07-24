#ifndef Config_hpp
#define Config_hpp

#define CONFIG_ENABLE_TRANSMITTER 1
#define CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL 0

namespace Config {
    enum class SensorId: uint8_t {little_room};
    constexpr uint8_t SENSOR_ID = uint8_t(SensorId::little_room);
}

#endif
