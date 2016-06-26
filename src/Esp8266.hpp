#ifndef Esp8266_hpp
#define Esp8266_hpp

#include <AltSoftSerial.h>

#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

class Esp8266: public Util::Task {
    public:
        typedef uint8_t ResponseInnerType;
        enum class Response: ResponseInnerType;

    private:
        enum class State: uint8_t;
        enum class Status: uint8_t;

        static constexpr TimeMillis COMMAND_TIMEOUT = 500;

    public:
        Esp8266(AltSoftSerial* serial, Util::TaskScheduler* scheduler);

    public:
        virtual void execute();

    private:
        void setState(State state);

        void onCheckEspConnection();
        void onConnectToServer();
        void onStartRequestSending();
        void onSendRequest();
        void onReceiveResponse();
        void onCloseConnection();
        void onProcessCommand();
        void onError();

        void sendCommand(const char* command, TimeMillis timeout=COMMAND_TIMEOUT);
        void sendCustomCommand(const char* command, TimeMillis timeout, bool raw, ResponseInnerType responseStopFlags);
        void waitCommandCompletion(TimeMillis timeout, ResponseInnerType responseStopFlags);
        void parseResponse();
        bool hasResponse(Response response);

    private:
        AltSoftSerial* serial_;
        char buf_[100]; // FIXME: size

        State state_;

        Status status_;
        ResponseInnerType responseStopFlags_;
        TimeMillis commandStartTime_;
        TimeMillis commandTimeout_;
        size_t responseSize_;
        ResponseInnerType response_;
};

#endif
