#ifndef Config_hpp
#define Config_hpp

#ifndef CONFIG_DEBUG_MODE
    #define CONFIG_DEBUG_MODE 0
#endif

#define CONFIG_ENABLE_TRANSMITTER 1
#define CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL 0
#define CONFIG_CO2_PWM_SENSOR_CYCLE_DURATION 1004000

namespace Config {
    enum class SensorId: uint8_t {little_room, big_room};
    constexpr uint8_t SENSOR_ID = uint8_t(SensorId::little_room);
    constexpr double CO2_PWM_SENSOR_TIME_SCALE_FACTOR = double(CONFIG_CO2_PWM_SENSOR_CYCLE_DURATION) / 1004000;
}

#endif
