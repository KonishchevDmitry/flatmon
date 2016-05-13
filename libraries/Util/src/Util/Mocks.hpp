#ifndef ARDUINO
#ifndef Util_Mocks_hpp
#define Util_Mocks_hpp

// Backported from Arduino.h
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

unsigned long millis();
void delay(unsigned long time);

namespace Util { namespace Mocks {

void setTime(unsigned long time);

}}

#endif
#endif
