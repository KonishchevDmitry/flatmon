void setup() {
    Serial.begin(9600);
}

void loop() {
    int result = Serial.read();
    if(result < 0)
        return;

    char character = char(result);
    Serial.print(character);

    if(character == '\r')
        Serial.print('\n');
}
