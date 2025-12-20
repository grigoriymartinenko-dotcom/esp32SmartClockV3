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
    _wasValid = false;
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

    bool anyChanged = false;

    if (getLocalTime(&_tm)) {
        _valid = true;
        _syncState = SYNCED;

        if (!_wasValid) {
            Serial.println("[NTP] Time synchronized");
            anyChanged = true; // первый валид — полный апдейт
        } else {
            if (_tm.tm_sec  != _prevTm.tm_sec)  anyChanged = true;
            if (_tm.tm_min  != _prevTm.tm_min)  anyChanged = true;
            if (_tm.tm_hour != _prevTm.tm_hour) anyChanged = true;
            if (_tm.tm_mday != _prevTm.tm_mday) anyChanged = true;
        }

        _prevTm = _tm;
        _wasValid = true;
    } else {
        _valid = false;

        if (_wasValid) {
            Serial.println("[NTP] Lost synchronization");
            anyChanged = true;
        }

        if (_syncState == SYNCING) {
            _syncState = ERROR;
        }

        _wasValid = false;
    }

    if (anyChanged) {
        _version.bump();
    }
}

const ServiceVersion& TimeService::version() const {
    return _version;
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