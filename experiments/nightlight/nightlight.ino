const int POT = 0;

const int RLED = 11;
const int GLED = 10;
const int BLED =  9;

const int COLORS = 16;
const int LED_NUM = 3;
const bool COMMON_ANODE = true;

const int PWM_HIGH = 255;
const int ANALOG_HIGH = 1023;

struct LedDensityRange {
    int led;
    int start;
    int end;
    int increment;

    LedDensityRange(int led, int start, int end)
    : led(led) {
        this->start = constrain(start, 0, COLORS - 1);
        this->end = constrain(end, 0, COLORS - 1);
        this->increment = this->start < this->end ? 1 : -1;
    }

    bool is_out_of(int density) const {
        if(start < end)
            return density < start || density > end;
        else
            return density < end || density > start;
    }
};

struct ColorRange {
    LedDensityRange leds[LED_NUM];
};

float get_brightness() {
    return 1 - float(analogRead(POT)) / ANALOG_HIGH;
}

void set_led_density(int led, int value, float brightness) {
    int pwm_low = 0;
    int pwm_high = PWM_HIGH * brightness;

    if(COMMON_ANODE) {
        pwm_low = PWM_HIGH;
        pwm_high = PWM_HIGH - pwm_high;
    }

    int density = map(value, 0, COLORS - 1, pwm_low, pwm_high);

    analogWrite(led, density);
}

void iterate_colors(const ColorRange& color_range) {
    int led_id = 0;
    int cursors[LED_NUM];

    cursors[led_id] = color_range.leds[led_id].start;

    while(true) {
        int density = cursors[led_id];
        const LedDensityRange& density_range = color_range.leds[led_id];

        if(density_range.is_out_of(density)) {
            if(led_id == 0)
                break;

            led_id--;
            cursors[led_id] += color_range.leds[led_id].increment;
        } else if(led_id < LED_NUM - 1) {
            led_id++;
            cursors[led_id] = color_range.leds[led_id].start;
        } else {
            float brightness = get_brightness();

            for(int led_id = 0; led_id < LED_NUM; led_id++) {
                set_led_density(color_range.leds[led_id].led, cursors[led_id], brightness);
            }

            cursors[led_id] += density_range.increment;

            delay(100);
        }
    }
}

void setup() {
    pinMode(RLED, OUTPUT);
    pinMode(GLED, OUTPUT);
    pinMode(BLED, OUTPUT);
}

void loop() {
    ColorRange color_ranges[] = {
        {{LedDensityRange(RLED, 15, 15), LedDensityRange(GLED, 15, 0),  LedDensityRange(BLED, 15, 0)}},  // R15 G15 B15 -> R15 G0 B0 (R)
        {{LedDensityRange(RLED, 15, 15), LedDensityRange(GLED, 1, 15),  LedDensityRange(BLED, 0, 0)}},   // R15 G1 B0   -> R15 G15 B0 (R->G)
        {{LedDensityRange(RLED, 14, 1),  LedDensityRange(GLED, 15, 15), LedDensityRange(BLED, 0, 0)}},   // R14 G15 B0  -> R1 G15 B0 (R->G)
        {{LedDensityRange(RLED, 0, 15),  LedDensityRange(GLED, 15, 15), LedDensityRange(BLED, 0, 15)}},  // R0 G15 B0   -> R15 G15 B15 (G)
        {{LedDensityRange(RLED, 15, 0),  LedDensityRange(GLED, 15, 0),  LedDensityRange(BLED, 15, 15)}}, // R15 G15 B15 -> R0 G0 B15 (B)
    };

    for(ColorRange& range : color_ranges) {
        iterate_colors(range);
    }
}
