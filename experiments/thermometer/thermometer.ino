#include <math.h>

#include <Util.h>
#include <Util/Constants.hpp>
#include <Util/CycleBuffer.hpp>

namespace Constants = Util::Constants;

const int TEMP_PIN = 0;

const int DATA_PIN = 10;
const int CLOCK_PIN = 8;
const int LATCH_PIN = 9;

// Attention: Use of tone() function interferes with PWM output on pins 3 and 11 (on boards other than the Mega).
const int SPEAKER_PIN = 11;

enum Comfort {
    COLD    = 1 << 3,
    NORM    = 1 << 2,
    WARM    = 1 << 1,
    HOT     = 1 << 0,
    UNKNOWN = 0,
};

class TemperatureSensor {
    public:
        bool measure() {
            values_.add(analogRead(TEMP_PIN));
            return values_.full();
        }

        float getTemperature() {
            float volts = float(values_.median()) / Constants::ANALOG_HIGH * Constants::VOLTS;
            return (volts - 0.5) * 100;
        }

    private:
        Util::CycleBuffer<uint16_t, 10> values_;
};

Comfort getComfort(float temperature) {
    long roundedTemperature = lround(temperature);

    if(roundedTemperature <= 18)
        return Comfort::COLD;
    else if(roundedTemperature <= 24)
        return Comfort::NORM;
    else if(roundedTemperature <= 26)
        return Comfort::WARM;
    else
        return Comfort::HOT;
}

const char* getComfortName(Comfort comfort) {
    switch(comfort) {
        case Comfort::COLD:
            return "cold";
        case Comfort::NORM:
            return "norm";
        case Comfort::WARM:
            return "warm";
        case Comfort::HOT:
            return "hot";
        default:
            return "unknown";
    }
}

void setLights(byte lights) {
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, lights);
    digitalWrite(LATCH_PIN, HIGH);
    digitalWrite(LATCH_PIN, LOW);
}

void notifyComfortChange() {
    tone(SPEAKER_PIN, 35);
    delay(100);
    tone(SPEAKER_PIN, 41, 50);
}

void setup() {
    Serial.begin(9600);

    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);

    pinMode(SPEAKER_PIN, OUTPUT);

    TemperatureSensor temperatureSensor;

    while(!temperatureSensor.measure()) {
        byte lights[] = {Comfort::COLD, Comfort::NORM, Comfort::WARM, Comfort::HOT};

        for(byte light : lights) {
            setLights(light);
            delay(100);
        }
    }

    Comfort comfort = Comfort::UNKNOWN;

    do {
        float temperature = temperatureSensor.getTemperature();
        Comfort curComfort = getComfort(temperature);

        if(curComfort != comfort) {
            comfort = curComfort;
            setLights(byte(comfort));
            notifyComfortChange();
        }

        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" (");
        Serial.print(getComfortName(comfort));
        Serial.println(")");

        delay(500);
        temperatureSensor.measure();
    } while(true);
}

void loop() {
}
