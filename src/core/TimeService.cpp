#include "core/TimeService.h"

void TimeService::begin() {
    _lastMs = millis();
}

void TimeService::update() {
    unsigned long now = millis();
    if (now - _lastMs >= 1000) {
        _lastMs += 1000;
        _s++;
        if (_s >= 60) {
            _s = 0;
            _m++;
            if (_m >= 60) {
                _m = 0;
                _h = (_h + 1) % 24;
            }
        }
    }
}

uint8_t TimeService::hour() const   { return _h; }
uint8_t TimeService::minute() const { return _m; }
uint8_t TimeService::second() const { return _s; }