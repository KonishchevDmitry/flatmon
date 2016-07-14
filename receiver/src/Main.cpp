#include <RH_ASK.h>

#include <Util.h>
#include <Util/Assertion.hpp>
#include <Util/Logging.hpp>

using Util::Logging::log;

#if 0
RH_ASK driver(2000, 8, A4, A5);

void setup() {
    Util::Logging::init();
    log(F("Initializing..."));

    {
        bool initialized = driver.init();
        UTIL_ASSERT(initialized);
    }

    while (true) {
        uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
        uint8_t buflen = sizeof(buf);
        if (driver.recv(buf, &buflen)) {
          buf[buflen] = 0;
          log(millis() / 1000, " > Message: ", (char*) buf);
        }
    }
}

void loop() {
    UTIL_LOGICAL_ERROR();
}
#else
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

RH_ASK driver(2000, 8, 9);

void setup()
{
    Serial.begin(9600);   // Debugging only
    if (!driver.init())
         Serial.println("init failed");
}

void loop()
{
    const char *msg = "Hello";
    TimeMicros startTime = micros();
    driver.send((uint8_t *)msg, strlen(msg));
    // driver.waitPacketSent();
    TimeMicros endtime = micros();
    log("Send time: ", endtime - startTime);
    delay(1000);
}
#endif
