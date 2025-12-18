#include <Arduino.h>
#include <time.h>
#include "services/TimeService.h"

void TimeService::begin() {
    _valid = false;
}

void TimeService::update() {
    // пытаемся получить системное время
    if (getLocalTime(&_tm, 100)) {   // ⬅️ таймаут 100 мс
        _valid = true;
    }
}

bool TimeService::isValid() const {
    return _valid;
}

int TimeService::hour() const {
    return _tm.tm_hour;
}

int TimeService::minute() const {
    return _tm.tm_min;
}

int TimeService::second() const {
    return _tm.tm_sec;
}