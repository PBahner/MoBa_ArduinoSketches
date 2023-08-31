#ifndef TURNOUTLEDS
#define TURNOUTLEDS

#include <ArduinoJson.h>
#include "expanderLed.h"

struct tPos {
    static const uint8_t plus = 0;
    static const uint8_t minus = 1;
    static const uint8_t none = 2;
};

class TurnoutLeds {
    public:
        TurnoutLeds(int id, MCP23017 *exp, uint8_t pinPlus, uint8_t pinMinus)
            : turnoutId(id), expander(exp), expPinPlus(pinPlus), expPinMinus(pinMinus) {};
        bool update(JsonObject &data) {
            if(!data.containsKey(String(turnoutId))) {return false;}
            turnoutPos = data[String(turnoutId)];
            updateLeds();
            return true;
        };

    private:
        uint8_t const turnoutId;
        MCP23017 *expander;
        uint8_t const expPinPlus;
        uint8_t const expPinMinus;
        uint8_t turnoutPos = tPos::none;
        ExpanderLED ledPlus = ExpanderLED(expander, &expPinPlus);
        ExpanderLED ledMinus = ExpanderLED(expander, &expPinMinus);

        void updateLeds() {
            if (turnoutPos == tPos::minus) {
                ledPlus.off();
                ledMinus.on();
            } else if (turnoutPos == tPos::plus) {
                ledMinus.off();
                ledPlus.on();
            } else {
                ledPlus.off();
                ledMinus.off();
            }
        }
};
#endif