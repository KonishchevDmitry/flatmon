// FIXME: A shitty-written work in progress mockup of communication with ESP8266

#include <Util/Assertion.hpp>
#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "Config.hpp"
#include "Esp8266.hpp"

using Util::Logging::log;
namespace Constants = Util::Constants;

enum class Esp8266::State: uint8_t {
    checking_connection, checking_ap_connection, connecting_to_ap, connecting_to_server, sending_data, closing_connection,
};

enum class Esp8266::Status: uint8_t {
    ready, command_in_process, command_processed,
};

constexpr TimeMillis SIMPLE_COMMAND_TIMEOUT = 5000; // FIXME

namespace {
    enum class Response: uint8_t;

    struct ResponseMapping {
        Response id;
        const char* text;
        uint8_t size;

        ResponseMapping(Response id, const char* text)
        : id(id), text(text), size(strlen(text)) {
        }
    };

    uint8_t operator|(Response id1, Response id2) {
        return uint8_t(id1) | uint8_t(id2);
    }

    uint8_t operator&(uint8_t response, Response id) {
        return response & uint8_t(id);
    }

    uint8_t operator|=(uint8_t& response, Response id) {
        return response |= uint8_t(id);
    }

    enum class Response: uint8_t {
        ok                = 1 << 0,
        error             = 1 << 1,

        already_connected = 1 << 2,
        connection_closed = 1 << 3,
        at_version        = 1 << 4,
    };

    ResponseMapping RESPONSES[] = {
        {Response::ok,    "OK"},
        {Response::error, "ERROR"},

        {Response::already_connected, "ALREADY CONNECTED"},
        {Response::connection_closed, "CLOSED"},
        {Response::at_version,        "AT version:0.40.0.0(Aug  8 2015 14:45:58)"}
    };
}

Esp8266::Esp8266(AltSoftSerial* serial, Util::TaskScheduler* scheduler)
: serial_(serial), state_(State::checking_connection), status_(Status::ready) {
    // FIXME: buffer sizes
    serial_->begin(9600);
    scheduler->addTask(this);
    this->scheduleAfter(3000); // FIXME
}

void Esp8266::setState(State state) {
    state_ = state;
    status_ = Status::ready;
}

void Esp8266::execute() {
    if(status_ == Status::command_in_process) {
        this->onProcessCommand();
        return;
    }

    // FIXME
    bool pending = status_ == Status::ready;

    if(state_ == State::checking_connection)
        this->onCheckConnection();
    else if(state_ == State::checking_ap_connection)
        this->onCheckApConnection(pending);
    else if(state_ == State::connecting_to_ap)
        this->onConnectToAp(pending);
    else if(state_ == State::connecting_to_server)
        this->onConnectToServer(pending);
    else if(state_ == State::sending_data)
        this->onSendData(pending);
    else if(state_ == State::closing_connection)
        this->onCloseConnection();
    else {
        UTIL_ASSERT(false);
    }
}

void Esp8266::onCheckConnection() {
    if(status_ == Status::ready) {
        log(F("Checking connection to ESP8266..."));
        return this->sendCommand("AT+GMR", SIMPLE_COMMAND_TIMEOUT);
    }

    if(response_ & Response::ok && response_ & Response::at_version) {
        log(F("ESP8266 connection works properly. Connecting to server..."));
        this->setState(State::connecting_to_server);
    } else {
        this->onError();
    }
}

// FIXME
void Esp8266::onCheckApConnection(bool pending) {
    /*
    if(pending) {
        // FIXME
        // this->sendCommand("AT+CWQAP", 0);
        return this->sendCommand("AT+CWJAP?", SIMPLE_COMMAND_TIMEOUT);
    }

    if(response_ & Response::ok) {
        if(response_ & Response::no_ap) {
            log(F("ESP8266 is not connected to AP. Connecting..."));
            state_ = State::connecting_to_ap;
            UTIL_ASSERT(false);
        } else {
            log(F("ESP8266 is connected to AP."));
            UTIL_ASSERT(false);
        }
    } else {
        UTIL_ASSERT(false); // FIXME
    }
    */
}

// FIXME
void Esp8266::onConnectToAp(bool pending) {
}

void Esp8266::onConnectToServer(bool pending) {
    if(pending) {
        // FIXME: iptables
        return this->sendCommand(R"(AT+CIPSTART="TCP","server.lan",8000)", SIMPLE_COMMAND_TIMEOUT);
    }

    if(response_ & Response::ok /* || response_ & Response::already_connected*/) {
        log(F("Connected to server. Sending data..."));
        this->setState(State::sending_data);
    } else {
        // FIXME
        this->setState(State::closing_connection);
    }
}

void Esp8266::onSendData(bool pending) {
    if(pending) {
        // FIXME: stop flags?
        this->sendCommand("AT+CIPSEND=8\r\n", SIMPLE_COMMAND_TIMEOUT);
        delay(2000);
        serial_->write("abcdefgh");
        return;
    }

    // if(response_ & Response::connection_closed) {
        log(F("Data successfuly sent to the server. Closing the connection..."));
        this->setState(State::closing_connection);
    // } else {
        // FIXME
        // this->onError();
    // }
}

void Esp8266::onCloseConnection() {
    if(status_ == Status::ready) {
        return this->sendCommand("AT+CIPCLOSE", SIMPLE_COMMAND_TIMEOUT);
    }

    // FIXME
    this->onError();
}

void Esp8266::onProcessCommand() {
    while(int data = serial_->read()) {
        if(data == -1)
            break;

        if(data == '\n')
            continue;

        if(data == '\r') {
            this->parseResponse();

            buf_[min(responseSize_, sizeof buf_ - 1)] = '\0';
            log(F("Got ESP8266 response: "), buf_);

            if(response_ & Response::error) {
                log(F("ESP8266 command failed. Execution time: "), millis() - commandStartTime_, F("."));
                status_ = Status::command_processed;
                return;
            }

            if(response_ & Response::ok || response_ & Response::connection_closed) {
                log(F("ESP8266 command succeeded. Execution time: "), millis() - commandStartTime_, F("."));
                status_ = Status::command_processed;
                return;
            }

            responseSize_ = 0;
            continue;
        }

        if(responseSize_ >= sizeof buf_) {
            buf_[min(responseSize_, sizeof buf_ - 1)] = '\0';
            log(F("Got truncated ESP8266 response: "), buf_);
            responseSize_ = 0;
        }

        buf_[responseSize_++] = data;
    }

    if(millis() - commandStartTime_ >= commandTimeout_) {
        log(F("ESP8266 command has timed out."));
        return this->onError();
    }

    // FIXME: delay
}

void Esp8266::onError() {
    this->setState(State::checking_connection);
    this->scheduleAfter(20 * Constants::SECOND_MILLIS);
}

void Esp8266::sendCommand(const char* command, TimeMillis timeout) {
    log(F("Sending ESP8266 command: "), command);

    // FIXME
    delay(1000);
    while(serial_->read() != -1)
        ;

    // FIXME: Check buffer is empty
    serial_->write(command);
    serial_->write("\r\n");

    commandStartTime_ = millis();
    commandTimeout_ = timeout;
    responseSize_ = 0;
    response_ = 0;

    status_ = Status::command_in_process;
    // FIXME: delay
}

void Esp8266::parseResponse() {
    if(!responseSize_)
        return;

    for(ResponseMapping& mapping : RESPONSES) {
        if(mapping.size == responseSize_ && !memcmp(mapping.text, buf_, mapping.size)) {
            response_ |= mapping.id;
            return;
        }
    }
}

#if 0
void setup() {
  /*
  espCommand("AT+CWQAP");
  */

  espCommand("AT+CWMODE=1");
  char buf[100];
  int resultSize = snprintf(buf, sizeof buf, R"(AT+CWJAP="%s","%s")", Config::AP_NAME, Config::AP_PASSWORD);
  UTIL_ASSERT(resultSize <= sizeof buf);
  espCommand(buf, 10000);
  espCommand("AT+CWJAP?");

#endif
