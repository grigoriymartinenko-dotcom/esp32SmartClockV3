#include "services/TimeService.h"

/*
 * TimeService
 * -----------
 * Единственный источник времени в системе.
 * Использует системный NTP ESP32.
 */

TimeService::TimeService() {}

void TimeService::begin() {
    _valid = false;
    _lastUpdateMs = 0;

    // Запуск NTP
    configTime(
        2 * 3600,   // UTC+2
        3600,       // DST
        "pool.ntp.org",
        "time.nist.gov"
    );
}

void TimeService::update() {
    unsigned long now = millis();
    if (now - _lastUpdateMs < UPDATE_INTERVAL)
        return;

    _lastUpdateMs = now;

    if (getLocalTime(&_tm)) {
        _valid = true;
    } else {
        _valid = false;
    }
}

bool TimeService::isValid() const { return _valid; }

int TimeService::hour()   const { return _valid ? _tm.tm_hour : 0; }
int TimeService::minute() const { return _valid ? _tm.tm_min  : 0; }
int TimeService::second() const { return _valid ? _tm.tm_sec  : 0; }

int TimeService::day()    const { return _valid ? _tm.tm_mday : 0; }
int TimeService::month() const { return _valid ? (_tm.tm_mon + 1) : 0; }
int TimeService::year()  const { return _valid ? (_tm.tm_year + 1900) : 0; }

int TimeService::weekday() const {
    return _valid ? _tm.tm_wday : 0;
}