#include <Util/Assertion.hpp>
#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "Esp8266.hpp"

using Util::Logging::log;
using Util::Logging::vlog;
using Util::Logging::vvlog;

namespace Constants = Util::Constants;

enum class Esp8266::State: uint8_t {
    check_esp_connection,
    connect_to_server,
    start_request_sending,
    send_request,
    receive_response,
    close_connection,
};
Esp8266::StateHandler Esp8266::stateHandlers_[] = {
    &Esp8266::onCheckEspConnection,
    &Esp8266::onConnectToServer,
    &Esp8266::onStartRequestSending,
    &Esp8266::onSendRequest,
    &Esp8266::onReceiveResponse,
    &Esp8266::onCloseConnection,
};

enum class Esp8266::Status: uint8_t {
    pending, in_process, processed,
};

namespace {
    typedef Esp8266::Response Response;
    typedef Esp8266::Responses Responses;

    class ResponseMapping {
        public:
            Response response;

        private:
            const char* text;
            uint8_t size;
            bool partial;

        public:
            ResponseMapping(Response response, const char* text, bool partial=false)
            : response(response), text(text), size(strlen(text)), partial(partial) {
            }

            bool matches(const char* responseLine, size_t size, bool partial) {
                return ((!partial && this->size == size) || (this->partial && this->size <= size)) &&
                       !memcmp(this->text, responseLine, this->size);
            }
    };

    Responses operator|(Response response1, Response response2) {
        return Responses(response1) | Responses(response2);
    }

    Responses operator&(Responses responses, Response response) {
        return responses & Responses(response);
    }

    Responses operator|=(Responses& responses, Response response) {
        return responses |= Responses(response);
    }
}

enum class Esp8266::Response: Responses {
    ok                = 1 << 0,
    error             = 1 << 1,
    timed_out         = 1 << 2,

    at_version        = 1 << 3,
    already_connected = 1 << 4,
    send_ok           = 1 << 5,
    connection_closed = 1 << 6,
};
static ResponseMapping RESPONSE_MAPPING[] = {
    {Response::ok,    "OK"},
    {Response::error, "ERROR"},

    {Response::at_version,        "AT version:0.40.0.0", true},
    {Response::already_connected, "ALREADY CONNECTED"},
    {Response::send_ok,           "SEND OK"},
    {Response::connection_closed, "CLOSED"},
};

Esp8266::Esp8266(Serial* serial, Util::TaskScheduler* scheduler)
: serial_(serial), connectionOpened_(false) {
    // FIXME: adjust buffer sizes
    serial_->begin(9600);
    scheduler->addTask(this);
    this->scheduleNextDataSending();
}

void Esp8266::setState(State state) {
    state_ = state;
    status_ = Status::pending;
}

void Esp8266::scheduleNextDataSending() {
    this->setState(State::check_esp_connection);
    this->scheduleAfter(this->DATA_SENDING_PERIOD);
}

void Esp8266::execute() {
    if(status_ == Status::in_process) {
        this->onProcessCommand();
    } else {
        size_t handlerId = size_t(state_);
        UTIL_ASSERT(handlerId < sizeof stateHandlers_ / sizeof *stateHandlers_);
        (this->*stateHandlers_[handlerId])();
    }
}

void Esp8266::onCheckEspConnection() {
    if(status_ == Status::pending) {
        log(F("Checking connection to ESP8266..."));
        return this->sendCommand("AT+GMR");
    }

    if(this->hasResponse(Response::ok) && this->hasResponse(Response::at_version)) {
        log(F("ESP8266 connection is working. Connecting to server..."));
        return this->setState(State::connect_to_server);
    }

    log(F("ESP8266 connection test failed."));
    this->onError();
}

void Esp8266::onConnectToServer() {
    // FIXME: port, iptables
    if(status_ == Status::pending)
        return this->sendCommand(R"(AT+CIPSTART="TCP","server.lan",8000)", 3 * Constants::SECOND_MILLIS);

    if(this->hasResponse(Response::ok)) {
        connectionOpened_ = true;
        log(F("Connected to server. Sending the request..."));
        this->setState(State::start_request_sending);
    } else if(this->hasResponse(Response::already_connected)) {
        connectionOpened_ = true;
        log(F("Connection to server is already opened. Aborting request sending..."));
        this->onError();
    } else {
        log(F("Failed to open connection to server."));
        this->onError();
    }
}

void Esp8266::onStartRequestSending() {
    if(status_ == Status::pending)
        return this->sendCommand("AT+CIPSEND=8");

    if(this->hasResponse(Response::ok)) {
        this->setState(State::send_request);
    } else {
        log(F("Failed to start request sending operation."));
        this->onError();
    }
}

void Esp8266::onSendRequest() {
    if(status_ == Status::pending)
        // FIXME: deprecate sendCustomCommand(raw=true)
        return this->sendCustomCommand("abcdefgh", COMMAND_TIMEOUT, true, Response::error | Response::send_ok);

    if(this->hasResponse(Response::send_ok)) {
        this->setState(State::receive_response);
    } else {
        log(F("Failed to send request to server."));
        this->onError();
    }
}

void Esp8266::onReceiveResponse() {
    // FIXME: check HTTP response
    if(status_ == Status::pending)
        return this->waitForCommandCompletion(5 * Constants::SECOND_MILLIS, Responses(Response::connection_closed));

    if(this->hasResponse(Response::connection_closed)) {
        connectionOpened_ = false;
        log(F("The request has been successfully sent."));
        this->scheduleNextDataSending();
    } else {
        log(F("Failed to receive response from server."));
        this->onError();
    }
}

void Esp8266::onError() {
    // FIXME: count sessions with errors
    if(connectionOpened_)
        this->setState(State::close_connection);
    else
        this->scheduleNextDataSending();
}

void Esp8266::onCloseConnection() {
    if(status_ == Status::pending) {
        connectionOpened_ = false;
        return this->sendCustomCommand("AT+CIPCLOSE", COMMAND_TIMEOUT, false,
                                       Response::connection_closed | Response::error);
    }

    if(this->hasResponse(Response::connection_closed)) {
        this->scheduleNextDataSending();
    } else {
        log(F("Failed to close connection to server."));
        this->onError();
    }
}

void Esp8266::onProcessCommand() {
    while(true) {
        int data = serial_->read();
        if(data == -1)
            break;

        if(data == '\r')
            continue;

        if(data == '\n') {
            if(this->onResponse())
                return;
            else
                continue;
        }

        if(responseSize_ < sizeof buf_)
            buf_[responseSize_] = data;

        ++responseSize_;
    }

    if(millis() - commandStartTime_ >= commandTimeout_) {
        vlog(F("ESP8266 command has timed out."));
        responses_ |= Response::timed_out;
        status_ = Status::processed;
        return;
    }

    this->scheduleAfter(this->COMMAND_EXECUTION_CHECK_PERIOD);
}

bool Esp8266::onResponse() {
    // FIXME: drop after buffer size adjustment
    static size_t maxResponseSize = 0;
    maxResponseSize = max(maxResponseSize, responseSize_);

    this->parseResponse();

    if(responseSize_ >= sizeof buf_) {
        buf_[sizeof buf_ - 1] = '\0';
        vvlog(F("ESP8266 (truncated): "), buf_);
    } else {
        buf_[responseSize_] = '\0';
        vvlog(F("ESP8266: "), buf_);
    }

    if(Responses commandProcessed = responses_ & commandProcessedResponses_) {
        #if UTIL_ENABLE_LOGGING && UTIL_LOG_VERBOSITY >= 1
            auto resultString = commandProcessed & Response::error ? F("failed") : F("succeeded");
            auto executionTime = millis() - commandStartTime_;
            vlog(F("ESP8266 command "), resultString, F(". Execution time: "), executionTime, F("."));
        #endif

        log("Max response size: ", maxResponseSize);
        status_ = Status::processed;
        return true;
    }

    responseSize_ = 0;
    return false;
}

void Esp8266::sendCommand(const char* command, TimeMillis timeout) {
    this->sendCustomCommand(command, timeout, false, Response::ok | Response::error);
}

void Esp8266::sendCustomCommand(const char* command, TimeMillis timeout, bool raw, Responses commandProcessedResponses) {
    // FIXME: drop after buffer size adjustment
    static size_t maxCommandSize = 0;
    maxCommandSize = max(maxCommandSize, strlen(command));
    log("Max command size: ", maxCommandSize);

    vlog(F("Sending ESP8266 command: "), command);

    // Flush all garbage from failed commands and boot messages.
    while(serial_->read() != -1)
        ;

    // FIXME: use async writes here
    serial_->write(command);
    // FIXME: deprecate raw mode
    if(!raw)
        serial_->write("\r\n");

    this->waitForCommandCompletion(timeout, commandProcessedResponses);
}

void Esp8266::waitForCommandCompletion(TimeMillis timeout, Responses commandProcessedResponses) {
    commandStartTime_ = millis();
    commandTimeout_ = timeout;
    commandProcessedResponses_ = commandProcessedResponses;
    responseSize_ = 0;
    responses_ = 0;

    status_ = Status::in_process;
    this->scheduleAfter(this->COMMAND_EXECUTION_CHECK_PERIOD);
}

void Esp8266::parseResponse() {
    if(!responseSize_)
        return;

    bool partial = responseSize_ > sizeof buf_;
    size_t dataSize = partial ? sizeof buf_ : responseSize_;

    for(ResponseMapping& mapping : RESPONSE_MAPPING) {
        if(mapping.matches(buf_, dataSize, partial)) {
            responses_ |= mapping.response;
            return;
        }
    }
}

bool Esp8266::hasResponse(Response response) {
    return responses_ & response;
}
