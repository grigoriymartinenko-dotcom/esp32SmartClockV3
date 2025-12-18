#include "core/NightService.h"

void NightService::setMode(Mode m) {
    _mode = m;
}

void NightService::update(uint8_t hour) {
    switch (_mode) {
        case Mode::DAY:
            _night = false;
            break;

        case Mode::NIGHT:
            _night = true;
            break;

        case Mode::AUTO:
            _night = (hour >= 22 || hour < 7);
            break;
    }
}

bool NightService::isNight() const {
    return _night;
}