#ifndef EXPANDERLED_H
#define EXPANDERLED_H

#include <MCP23017.h>
#include <ArduinoJson.h>

class ExpanderLED {
    public:
        ExpanderLED() {}
        ExpanderLED(MCP23017 *exp, uint8_t const *pin)
            : expander(exp), expPin(*pin) {
        };

        void begin() {
            began = true;
            expander->pinMode(expPin, OUTPUT);
            expander->digitalWrite(expPin, LOW);
        };

        void begin(MCP23017 *exp, uint8_t *pin) {
            began = true;
            expander = exp;
            expPin = *pin;
            expander->pinMode(expPin, OUTPUT);
            expander->digitalWrite(expPin, LOW);
        };

        bool update(JsonArray *data);

        bool set(bool state) {
            if (state) {
                on();
            } else {
                off();
            }
            return ledState;
        }

        void on() {
            if(!began or ledState) {return;}
            expander->digitalWrite(expPin, HIGH);
            ledState = HIGH;
        };

        void off() {
            if(!began or !ledState) {return;}
            expander->digitalWrite(expPin, LOW);
            ledState = LOW;
        };

    private:
        MCP23017 *expander;
        uint8_t expPin;
        bool ledState = LOW;
        bool began = false;
};
#endif