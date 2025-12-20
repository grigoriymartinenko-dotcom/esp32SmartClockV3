#include "services/TimeService.h"
#include <Arduino.h>

TimeService::TimeService(UiVersionService& uiVersion)
    : _uiVersion(uiVersion)
{
}

void TimeService::begin() {
    syncNtp();
}

void TimeService::setTimezone(long gmtOffsetSec, int daylightOffsetSec) {
    _gmtOffsetSec = gmtOffsetSec;
    _daylightOffsetSec = daylightOffsetSec;
    configTime(_gmtOffsetSec, _daylightOffsetSec, "pool.ntp.org");
}

void TimeService::update() {
    updateTime();
}

void TimeService::updateTime() {
    if (!getLocalTime(&_timeinfo)) {
        _syncState = ERROR;
        return;
    }

    _syncState = SYNCED;

    // 🔹 логическое событие
    if (_timeinfo.tm_min != _lastMinute) {
        _lastMinute = _timeinfo.tm_min;
        _uiVersion.bump(UiChannel::TIME);
    }
    static int lastSecond = -1;

if (_timeinfo.tm_sec != lastSecond) {
    lastSecond = _timeinfo.tm_sec;
    _uiVersion.bump(UiChannel::TIME); // 🔥 тик для blink
}
}

void TimeService::syncNtp() {
    _syncState = SYNCING;
}

int TimeService::hour() const {
    return _timeinfo.tm_hour;
}

int TimeService::minute() const {
    return _timeinfo.tm_min;
}

int TimeService::second() const {
    return _timeinfo.tm_sec;
}

// ===== COMPATIBILITY =====

bool TimeService::isValid() const {
    return _syncState == SYNCED;
}

int TimeService::day() const {
    return _timeinfo.tm_mday;
}

int TimeService::month() const {
    return _timeinfo.tm_mon + 1;
}

int TimeService::year() const {
    return _timeinfo.tm_year + 1900;
}

TimeService::SyncState TimeService::syncState() const {
    return _syncState;
}