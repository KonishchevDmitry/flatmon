#include "pitches.h"

const int SPEAKER = 9;

struct Sound {
    int frequency;
    int duration;
};

const Sound SOUNDS[] = {
    {NOTE_A4, 250}, {NOTE_E4, 250}, {NOTE_A4, 250}, {0, 250},
    {NOTE_A4, 250}, {NOTE_E4, 250}, {NOTE_A4, 250}, {0, 250},
    {NOTE_E4, 125}, {NOTE_D4, 125}, {NOTE_C4, 125}, {NOTE_B4, 125},
    {NOTE_A4, 125}, {NOTE_B4, 125}, {NOTE_C4, 125}, {NOTE_D4, 125},
    {NOTE_E4, 250}, {NOTE_E3, 250}, {NOTE_A4, 250}, {0, 250},
};

void setup() {
}

void loop() {
    for(const Sound& sound : SOUNDS) {
        tone(SPEAKER, sound.frequency, sound.duration);
        delay(sound.duration);
    }
}
