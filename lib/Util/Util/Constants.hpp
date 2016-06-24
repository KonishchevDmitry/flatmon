#ifndef Util_Constants_hpp
#define Util_Constants_hpp

#include "Core.hpp"

namespace Util { namespace Constants {

constexpr int VOLTS = 5;

constexpr int PWM_LOW = 0;
constexpr int PWM_HIGH = 255;

constexpr int ANALOG_LOW = 0;
constexpr int ANALOG_HIGH = 1023;

constexpr TimeMillis SECOND_MILLIS = 1000;
constexpr TimeMillis MINUTE_MILLIS = 60 * SECOND_MILLIS;

}}

#endif
