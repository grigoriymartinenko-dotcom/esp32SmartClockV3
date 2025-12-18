#pragma once
#include <Arduino.h>

/*
 * NightService
 * ============
 * DAY / NIGHT / AUTO
 */
class NightService {
public:
    enum class Mode : uint8_t {
        DAY,
        NIGHT,
        AUTO
    };

    void setMode(Mode m);
    void update(uint8_t hour);

    bool isNight() const;

private:
    Mode _mode = Mode::AUTO;
    bool _night = false;
};