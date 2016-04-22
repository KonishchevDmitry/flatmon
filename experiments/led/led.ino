const int LED = 9;

void blink_led() {
    for(int period = 100; period <= 1000; period += 100) {
        digitalWrite(LED, HIGH);
        delay(period);

        digitalWrite(LED, LOW);
        delay(period);
    }

}

void fade_led() {
    int brightness = 0;

    while(brightness <= 255) {
        analogWrite(LED, brightness);
        brightness++;
        delay(10);
    }

    while(brightness >= 0) {
        analogWrite(LED, brightness);
        brightness--;
        delay(10);
    }
}

void setup() {
    pinMode(LED, OUTPUT);
}

void loop() {
    blink_led();

    for(int i = 0; i < 10; i++) {
        fade_led();
    }
}
