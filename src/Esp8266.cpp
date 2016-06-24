#include <Util/Assertion.hpp>
#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "Config.hpp"
#include "Esp8266.hpp"

using Util::Logging::log;
namespace Constants = Util::Constants;

enum class Esp8266::State: uint8_t {none};

Esp8266::Esp8266(AltSoftSerial* serial, Util::TaskScheduler* scheduler)
: serial_(serial), state_(State::none) {
    serial_->begin(9600);
    scheduler->addTask(this);
}

void Esp8266::execute() {
    switch(state_) {
        case State::none:
            break;
        default:
            UTIL_ASSERT(false);
            break;
    }
}

// FIXME: A shitty-written mockup of communication with ESP8266
#if 0
void espCommand(const char* command, int timeout = 2000) {
    int tries = 3;
    char buf[500];
    size_t size = 0;
    while(tries-- > 0) {
        size = 0;
        log("Command: ", command);

        SOFTWARE_SERIAL.write(command);
        SOFTWARE_SERIAL.write("\r\n");

        unsigned long startTime = millis();
        while(millis() - startTime < timeout && size < sizeof buf) {
            int data = SOFTWARE_SERIAL.read();
            if(data != -1) {
                buf[size++] = data;
            }
        }

        char okResponse[] = {'O', 'K', '\r', '\n'};
        if(size >= sizeof okResponse && !memcmp(buf + size - sizeof okResponse, okResponse, sizeof okResponse)) {
            buf[size] = '\0';
            log("Command '", command, "' succeded:\n", buf);
            return;
        }

        buf[size] = '\0';
        log("Command '", command, "' failed:\n", buf);
    }

    UTIL_ASSERT(false);
}

void setup() {
  Util::Logging::init();
  SOFTWARE_SERIAL.begin(9600);
  // Serial.begin(9600);
  // Serial.println("Setup done");
  // delay(5000);
  // log("sending command");
  log("started");

  espCommand("AT");
  /*
  espCommand("AT+CWQAP");
  */

  espCommand("AT+CWMODE=1");
  char buf[100];
  int resultSize = snprintf(buf, sizeof buf, R"(AT+CWJAP="%s","%s")", Config::AP_NAME, Config::AP_PASSWORD);
  UTIL_ASSERT(resultSize <= sizeof buf);
  espCommand(buf, 10000);
  espCommand("AT+CWJAP?");

  #if 0
  espCommand(R"(AT+CIPSTART="TCP","192.168.0.1",80)");
  espCommand("AT+CIPSEND=3");
  espCommand("d\r\n");
  espCommand("AT+CIPCLOSE");
  #endif

  // delay(5000);
  // log("Sending command");
  // SOFTWARE_SERIAL.write("AT\r\n");
  // log("command sent");
}
#endif
