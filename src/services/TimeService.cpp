#include "services/TimeService.h"
#include <Arduino.h>

TimeService::TimeService(UiVersionService& uiVersion)
    : _uiVersion(uiVersion)
{
}

void TimeService::begin() {
    syncNtp();   // NTP стартует, но НЕ блокирует
}

void TimeService::setTimezone(long gmtOffsetSec, int daylightOffsetSec) {
    _gmtOffsetSec = gmtOffsetSec;
    _daylightOffsetSec = daylightOffsetSec;
    configTime(_gmtOffsetSec, _daylightOffsetSec, "pool.ntp.org");
}

void TimeService::setFromRtc(const tm& t) {
    _timeinfo = t;
    _valid = true;
    _source = RTC;

    _lastMinute = t.tm_min;
    _lastSecond = t.tm_sec;

    _uiVersion.bump(UiChannel::TIME);
}

void TimeService::update() {
    updateTime();
}

void TimeService::updateTime() {

    tm t;
    if (!getLocalTime(&t)) {
        if (!_valid) {
            _syncState = ERROR;
        }
        return;
    }

    _timeinfo = t;
    _valid = true;
    _source = NTP;
    _syncState = SYNCED;

    if (t.tm_min != _lastMinute) {
        _lastMinute = t.tm_min;
        _uiVersion.bump(UiChannel::TIME);
    }

    if (t.tm_sec != _lastSecond) {
        _lastSecond = t.tm_sec;
        _uiVersion.bump(UiChannel::TIME); // blink
    }
}

void TimeService::syncNtp() {
    _syncState = SYNCING;
}

bool TimeService::isValid() const {
    return _valid;
}

int TimeService::hour() const   { return _timeinfo.tm_hour; }
int TimeService::minute() const { return _timeinfo.tm_min; }
int TimeService::second() const { return _timeinfo.tm_sec; }

int TimeService::day() const   { return _timeinfo.tm_mday; }
int TimeService::month() const { return _timeinfo.tm_mon + 1; }
int TimeService::year() const  { return _timeinfo.tm_year + 1900; }

TimeService::SyncState TimeService::syncState() const {
    return _syncState;
}

TimeService::Source TimeService::source() const {
    return _source;
}

bool TimeService::getTm(tm& out) const {
    if (!_valid) return false;
    out = _timeinfo;
    return true;
}