const int LED = 9;
const int BUTTON = 2;

bool button_was_pressed() {
    static bool button_state = LOW;

    if(digitalRead(BUTTON) == button_state)
        return false;

    delay(5);

    if(digitalRead(BUTTON) == button_state)
        return false;

    button_state = !button_state;

    return button_state;
}

void setup() {
    pinMode(LED, OUTPUT);
    pinMode(BUTTON, INPUT);
}

void loop() {
    static bool led_state = LOW;

    if(button_was_pressed()) {
        led_state = !led_state;
        digitalWrite(LED, led_state);
    }
}
