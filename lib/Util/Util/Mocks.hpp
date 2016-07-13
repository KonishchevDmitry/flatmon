#ifndef ARDUINO
#ifndef Util_Mocks_hpp
#define Util_Mocks_hpp

#include <algorithm>

#include "Core.hpp"

using std::min;
using std::max;

unsigned long millis();
void delay(unsigned long time);

namespace Util { namespace Mocks {

void setTime(TimeMillis time);

}}

#endif
#endif
