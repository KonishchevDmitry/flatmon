#ifndef Esp8266_hpp
#define Esp8266_hpp

#include <AltSoftSerial.h>

#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

class Esp8266: public Util::Task {
    private:
        enum class State: uint8_t;
        enum class Status: uint8_t;

    public:
        Esp8266(AltSoftSerial* serial, Util::TaskScheduler* scheduler);

    public:
        virtual void execute();

    private:
        void setState(State state);

        void onCheckConnection();
        void onCheckApConnection(bool pending);
        void onConnectToAp(bool pending);
        void onConnectToServer(bool pending);
        void onCloseConnection();
        void onSendData(bool pending);
        void onProcessCommand();
        void onError();

        void sendCommand(const char* command, TimeMillis timeout);
        void parseResponse();

    private:
        AltSoftSerial* serial_;
        char buf_[100]; // FIXME: size

        State state_;

        Status status_;
        TimeMillis commandStartTime_;
        TimeMillis commandTimeout_;
        size_t responseSize_;
        uint8_t response_;
};

#endif
