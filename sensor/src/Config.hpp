#ifndef Config_hpp
#define Config_hpp

#define CONFIG_ENABLE_TRANSMITTER 0

#define CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL !CONFIG_ENABLE_TRANSMITTER
#define CONFIG_ENABLE_CO2_SENSOR (CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL || !UTIL_ENABLE_LOGGING)

// FIXME
#define CONFIG_ENABLE_CO2_SENSOR_PWM_EXPERIMENT 0

namespace Config {
    enum class SensorId: uint8_t {little_room};
    constexpr uint8_t SENSOR_ID = uint8_t(SensorId::little_room);
}

#endif
