#include "services/TimeService.h"

TimeService::TimeService() {}

void TimeService::setTimezone(int utcOffset, int dst) {
    _utcOffset = utcOffset;
    _dstOffset = dst;
}

void TimeService::applyTimezone() {
    configTime(
        _utcOffset,
        _dstOffset,
        "pool.ntp.org",
        "time.nist.gov"
    );
}

void TimeService::begin() {
    _valid = false;
    _lastUpdateMs = 0;

    _syncState = SYNCING;
    Serial.println("[NTP] Starting synchronization...");

    applyTimezone();
}

void TimeService::update() {
    unsigned long now = millis();
    if (now - _lastUpdateMs < UPDATE_INTERVAL)
        return;

    _lastUpdateMs = now;

    bool wasValid = _valid;

    if (getLocalTime(&_tm)) {
        _valid = true;
        _syncState = SYNCED;

        if (!wasValid) {
            Serial.println("[NTP] Time synchronized");
        }
    } else {
        _valid = false;

        if (wasValid) {
            Serial.println("[NTP] Lost synchronization");
        }

        if (_syncState == SYNCING) {
            _syncState = ERROR;
        }
    }
}

TimeService::SyncState TimeService::syncState() const {
    return _syncState;
}

bool TimeService::isValid() const {
    return _valid;
}

// ===== TIME =====
int TimeService::hour()   const { return _valid ? _tm.tm_hour : 0; }
int TimeService::minute() const { return _valid ? _tm.tm_min  : 0; }
int TimeService::second() const { return _valid ? _tm.tm_sec  : 0; }

// ===== DATE =====
int TimeService::day()   const { return _valid ? _tm.tm_mday : 0; }
int TimeService::month() const { return _valid ? (_tm.tm_mon + 1) : 0; }
int TimeService::year()  const { return _valid ? (_tm.tm_year + 1900) : 0; }

// ===== WEEKDAY =====
TimeService::Weekday TimeService::weekday() const {
    return _valid
        ? static_cast<Weekday>(_tm.tm_wday)
        : SUN;
}