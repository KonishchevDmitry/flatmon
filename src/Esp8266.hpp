#ifndef Esp8266_hpp
#define Esp8266_hpp

#include <AltSoftSerial.h>

#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

class Esp8266: public Util::Task {
    public:
        typedef uint8_t Responses;
        enum class Response: Responses;

    private:
        typedef AltSoftSerial Serial;

        enum class State: uint8_t;
        enum class Status: uint8_t;

        typedef void (Esp8266::* StateHandler)();

        static constexpr TimeMillis DATA_SENDING_PERIOD = Util::Constants::MINUTE_MILLIS / 6; // FIXME
        static constexpr TimeMillis COMMAND_EXECUTION_CHECK_PERIOD = 0; // FIXME: Adjust by average command execution time
        static constexpr TimeMillis COMMAND_TIMEOUT = 500;

    public:
        Esp8266(Serial* serial, Util::TaskScheduler* scheduler);

    public:
        virtual void execute();

    private:
        void setState(State state);
        void scheduleNextDataSending();

        void onCheckEspConnection();
        void onConnectToServer();
        void onStartRequestSending();
        void onSendRequest();
        void onReceiveResponse();
        void onCloseConnection();
        void onProcessCommand();
        bool onResponse();
        void onError();

        void sendCommand(const char* command, TimeMillis timeout=COMMAND_TIMEOUT);
        void sendCustomCommand(const char* command, TimeMillis timeout, bool raw, Responses commandProcessedResponses);
        void waitForCommandCompletion(TimeMillis timeout, Responses commandProcessedResponses);
        void parseResponse();
        bool hasResponse(Response response);

    private:
        Serial* serial_;

        State state_;
        Status status_;
        bool connectionOpened_;

        TimeMillis commandTimeout_;
        TimeMillis commandStartTime_;
        Responses commandProcessedResponses_;

        char buf_[100]; // FIXME: adjust size
        size_t responseSize_;
        Responses responses_;

        static StateHandler stateHandlers_[];
};

#endif
