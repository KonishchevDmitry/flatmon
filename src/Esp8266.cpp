// FIXME: A shitty-written work in progress mockup of communication with ESP8266

#include <Util/Assertion.hpp>
#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "Esp8266.hpp"

// FIXME: check all below
using Util::Logging::log;
using Util::Logging::vlog;
using Util::Logging::vvlog;

namespace Constants = Util::Constants;

enum class Esp8266::State: uint8_t {
    checking_esp_connection,
    connecting_to_server,
    starting_request_sending,
    sending_request,
    receiving_response,
    closing_connection,
};

enum class Esp8266::Status: uint8_t {
    ready, command_in_process, command_processed,
};

typedef Esp8266::Response Response;
typedef Esp8266::ResponseInnerType ResponseInnerType;

namespace {
    struct ResponseMapping {
        Response id;
        const char* text;
        uint8_t size;

        ResponseMapping(Response id, const char* text)
        : id(id), text(text), size(strlen(text)) {
        }
    };

    ResponseInnerType operator|(Response id1, Response id2) {
        return ResponseInnerType(id1) | ResponseInnerType(id2);
    }

    ResponseInnerType operator&(ResponseInnerType response, Response id) {
        return response & ResponseInnerType(id);
    }

    ResponseInnerType operator|=(ResponseInnerType& response, Response id) {
        return response |= ResponseInnerType(id);
    }

}

enum class Esp8266::Response: ResponseInnerType {
    ok                = 1 << 0,
    error             = 1 << 1,
    timed_out         = 1 << 2,

    already_connected = 1 << 3,
    connection_closed = 1 << 4,
    at_version        = 1 << 5,
    send_ok           = 1 << 6,
};
static ResponseMapping RESPONSES[] = {
    {Response::ok,    "OK"},
    {Response::error, "ERROR"},

    {Response::already_connected, "ALREADY CONNECTED"},
    {Response::connection_closed, "CLOSED"},
    {Response::at_version,        "AT version:0.40.0.0(Aug  8 2015 14:45:58)"},
    {Response::send_ok,           "SEND OK"},
};

Esp8266::Esp8266(AltSoftSerial* serial, Util::TaskScheduler* scheduler)
: serial_(serial), state_(State::checking_esp_connection), status_(Status::ready) {
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
    if(status_ == Status::command_in_process)
        return this->onProcessCommand();

    if(state_ == State::checking_esp_connection)
        this->onCheckEspConnection();
    else if(state_ == State::connecting_to_server)
        this->onConnectToServer();
    else if(state_ == State::starting_request_sending)
        this->onStartRequestSending();
    else if(state_ == State::sending_request)
        this->onSendRequest();
    else if(state_ == State::receiving_response)
        this->onReceiveResponse();
    else if(state_ == State::closing_connection)
        this->onCloseConnection();
    else
        UTIL_LOGICAL_ERROR();
}

void Esp8266::onCheckEspConnection() {
    if(status_ == Status::ready) {
        log(F("Checking connection to ESP8266..."));
        return this->sendCommand("AT+GMR");
    }

    if(this->hasResponse(Response::ok) && this->hasResponse(Response::at_version)) {
        log(F("ESP8266 connection has established. Connecting to server..."));
        return this->setState(State::connecting_to_server);
    }

    log(F("ESP8266 connection test failed."));
    this->onError();
}

void Esp8266::onConnectToServer() {
    // FIXME: iptables
    if(status_ == Status::ready)
        return this->sendCommand(R"(AT+CIPSTART="TCP","server.lan",8000)", 3 * Constants::SECOND_MILLIS);

    if(this->hasResponse(Response::ok)) {
        log(F("Connected to server. Sending the request..."));
        this->setState(State::starting_request_sending);
    } else if(this->hasResponse(Response::already_connected)) {
        log(F("Connection to server is already opened. Aborting request sending..."));
        this->setState(State::closing_connection);
    } else {
        log(F("Failed to open connection to server."));
        this->onError();
    }
}

void Esp8266::onStartRequestSending() {
    if(status_ == Status::ready)
        return this->sendCommand("AT+CIPSEND=8");

    if(this->hasResponse(Response::ok)) {
        this->setState(State::sending_request);
    } else {
        log(F("Failed to establish connection to server. Aborting request sending..."));
        this->setState(State::closing_connection);
    }
}

void Esp8266::onSendRequest() {
    if(status_ == Status::ready)
        return this->sendCustomCommand("abcdefgh", COMMAND_TIMEOUT, true, Response::error | Response::send_ok);

    if(this->hasResponse(Response::send_ok)) {
        this->setState(State::receiving_response);
    } else {
        log(F("Failed to send request to server."));
        this->setState(State::closing_connection);
    }
}

void Esp8266::onReceiveResponse() {
    if(status_ == Status::ready)
        return this->waitForCommandCompletion(5 * Constants::SECOND_MILLIS, ResponseInnerType(Response::connection_closed));

    if(this->hasResponse(Response::connection_closed)) {
        log(F("The request has been successfully sent."));
        this->onError(); // FIXME
    } else {
        log(F("Failed to receive response from server."));
        this->setState(State::closing_connection);
    }
}

void Esp8266::onCloseConnection() {
    if(status_ == Status::ready) {
        return this->sendCommand("AT+CIPCLOSE");
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
            vvlog(F("ESP8266: "), buf_);

            if(response_ & responseStopFlags_) {
                if(response_ & responseStopFlags_ & Response::error)
                    vlog(F("ESP8266 command failed. Execution time: "), millis() - commandStartTime_, F("."));
                else
                    vlog(F("ESP8266 command succeeded. Execution time: "), millis() - commandStartTime_, F("."));

                status_ = Status::command_processed;
                return;
            }

            responseSize_ = 0;
            continue;
        }

        if(responseSize_ >= sizeof buf_) {
            buf_[min(responseSize_, sizeof buf_ - 1)] = '\0';
            vlog(F("Got truncated ESP8266 response: "), buf_);
            responseSize_ = 0;
        }

        buf_[responseSize_++] = data;
    }

    if(millis() - commandStartTime_ >= commandTimeout_) {
        vlog(F("ESP8266 command has timed out."));
        response_ |= Response::timed_out;
        status_ = Status::command_processed;
        return;
    }

    // FIXME: delay
}

void Esp8266::onError() {
    this->setState(State::checking_esp_connection);
    this->scheduleAfter(20 * Constants::SECOND_MILLIS);
}

void Esp8266::sendCommand(const char* command, TimeMillis timeout) {
    this->sendCustomCommand(command, timeout, false, Response::ok | Response::error);
}

void Esp8266::sendCustomCommand(const char* command, TimeMillis timeout, bool raw, ResponseInnerType responseStopFlags) {
    vlog(F("Sending ESP8266 command: "), command);

    // FIXME
    delay(1000);
    while(serial_->read() != -1)
        ;

    // FIXME: Check buffer size
    serial_->write(command);
    if(!raw)
        serial_->write("\r\n");

    this->waitForCommandCompletion(timeout, responseStopFlags);
}

void Esp8266::waitForCommandCompletion(TimeMillis timeout, ResponseInnerType responseStopFlags) {
    commandStartTime_ = millis();
    commandTimeout_ = timeout;
    responseStopFlags_ = responseStopFlags;
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

bool Esp8266::hasResponse(Response response) {
    return response_ & response;
}
