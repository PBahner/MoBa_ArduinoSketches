#ifndef SIGNALLED_H
#define SIGNALLED_H

#include "expanderLed.h"


class SignalLED : public ExpanderLED {
    public:
        SignalLED( int trackId, int section, MCP23017 *exp, uint8_t const pin)
            : trackId(trackId), section(section), ExpanderLED(exp, &pin) {}

        int getTrackId() const {
            return trackId;
        }

        bool update(JsonObject &data) {
            uint16_t interruptionKey = (trackId << 2) + section;
            if(!data.containsKey(String(interruptionKey))) {
                off();
                return false;
            } else {
                on();
                return true;
            }
        };

    private:
        uint8_t trackId;
        uint8_t section;
};
#endif