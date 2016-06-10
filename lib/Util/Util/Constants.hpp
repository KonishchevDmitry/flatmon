#ifndef Util_Constants_hpp
#define Util_Constants_hpp

#include "Core.hpp"

namespace Util { namespace Constants {

static const int VOLTS = 5;

static const int PWM_LOW = 0;
static const int PWM_HIGH = 255;

static const int ANALOG_LOW = 0;
static const int ANALOG_HIGH = 1023;

static const unsigned long SECOND_MILLIS = 1000;
static const unsigned long MINUTE_MILLIS = 60 * SECOND_MILLIS;

}}

#endif
