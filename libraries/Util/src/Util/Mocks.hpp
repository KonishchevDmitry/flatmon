#ifndef ARDUINO
#ifndef Util_Mocks_hpp
#define Util_Mocks_hpp

unsigned long millis();
void delay(unsigned long time);

namespace Util { namespace Mocks {

void setTime(unsigned long time);

}}

#endif
#endif
