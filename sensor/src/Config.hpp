#ifndef Config_hpp
#define Config_hpp

#define CONFIG_ENABLE_TRANSMITTER 0
#define CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL 0
#define CONFIG_CO2_PWM_SENSOR_CYCLE_DURATION 1004408

namespace Config {
    enum class SensorId: uint8_t {little_room};
    constexpr uint8_t SENSOR_ID = uint8_t(SensorId::little_room);
    constexpr double CO2_PWM_SENSOR_TIME_SCALE_FACTOR = double(CONFIG_CO2_PWM_SENSOR_CYCLE_DURATION) / 1004000;
}

#endif
