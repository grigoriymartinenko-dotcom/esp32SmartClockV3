#include "services/TimeService.h"
#include <Arduino.h>
#include <sys/time.h>

// ------------------------------------------------------------
// ctor
// ------------------------------------------------------------
TimeService::TimeService(UiVersionService& uiVersion)
    : _uiVersion(uiVersion)
{}

// ------------------------------------------------------------
// begin
// ------------------------------------------------------------
void TimeService::begin() {
    _syncState = NOT_STARTED;
    _ntpConfirmed = false;

    if (_mode == AUTO || _mode == NTP_ONLY) {
        syncNtp();
    }
}

// ------------------------------------------------------------
// update
// ------------------------------------------------------------
void TimeService::update() {
    updateTime();
}

// ------------------------------------------------------------
// MODE
// ------------------------------------------------------------
void TimeService::setMode(Mode m) {
    if (_mode == m)
        return;

    _mode = m;

    _syncState    = NOT_STARTED;
    _ntpConfirmed = false;

    if (_mode == AUTO || _mode == NTP_ONLY) {
        syncNtp();
    }

    if (_mode == LOCAL_ONLY) {
        _source = NONE;
    }

    _uiVersion.bump(UiChannel::TIME);
}

TimeService::Mode TimeService::mode() const {
    return _mode;
}

// ------------------------------------------------------------
// TIMEZONE
// ------------------------------------------------------------
void TimeService::setTimezone(long gmtOffsetSec, int daylightOffsetSec) {
    _gmtOffsetSec      = gmtOffsetSec;
    _daylightOffsetSec = daylightOffsetSec;

    // DST управляем вручную
    configTime(_gmtOffsetSec, 0, "pool.ntp.org");
}

// ------------------------------------------------------------
// RTC
// ------------------------------------------------------------
void TimeService::setFromRtc(const tm& t) {
    if (_mode == NTP_ONLY || _mode == LOCAL_ONLY)
        return;

    _timeinfo = t;
    _valid    = true;
    _source   = RTC;

    tm tmp = t;
    time_t epoch = mktime(&tmp);
    if (epoch > 0) {
        timeval tv{};
        tv.tv_sec = epoch;
        tv.tv_usec = 0;
        settimeofday(&tv, nullptr);
    }

    _uiVersion.bump(UiChannel::TIME);
}

// ------------------------------------------------------------
// updateTime
// ------------------------------------------------------------
void TimeService::updateTime() {

    if (_mode == LOCAL_ONLY)
        return;

    tm t{};
    if (!getLocalTime(&t)) {
        if (_mode == NTP_ONLY)
            _syncState = ERROR;
        return;
    }

    _timeinfo = t;
    _valid    = true;

    // подтверждение NTP
    if ((_mode == AUTO || _mode == NTP_ONLY) &&
        _syncState == SYNCING && !_ntpConfirmed) {

        _ntpConfirmed = true;
        _syncState    = SYNCED;
        _source       = NTP;
    }

    // AUTO → пока нет NTP, считаем RTC
    if (_mode == AUTO && !_ntpConfirmed) {
        _source = RTC;
    }

    // DST
    bool newDst = _dst.isDst(t);
    if (newDst != _dstActive) {
        _dstActive = newDst;
        configTime(
            _gmtOffsetSec,
            _dstActive ? _daylightOffsetSec : 0,
            "pool.ntp.org"
        );
        _uiVersion.bump(UiChannel::TIME);
    }

    // обновление UI
    if (t.tm_min != _lastMinute || t.tm_sec != _lastSecond) {
        _lastMinute = t.tm_min;
        _lastSecond = t.tm_sec;
        _uiVersion.bump(UiChannel::TIME);
    }
}

// ------------------------------------------------------------
// NTP
// ------------------------------------------------------------
void TimeService::syncNtp() {
    _syncState = SYNCING;
}

// ------------------------------------------------------------
// getters
// ------------------------------------------------------------
bool TimeService::isValid() const { return _valid; }

int TimeService::hour()   const { return _timeinfo.tm_hour; }
int TimeService::minute() const { return _timeinfo.tm_min;  }
int TimeService::second() const { return _timeinfo.tm_sec;  }

int TimeService::day()    const { return _timeinfo.tm_mday; }
int TimeService::month()  const { return _timeinfo.tm_mon + 1; }
int TimeService::year()   const { return _timeinfo.tm_year + 1900; }

TimeService::SyncState TimeService::syncState() const { return _syncState; }
TimeService::Source    TimeService::source()    const { return _source; }

bool TimeService::getTm(tm& out) const {
    if (!_valid) return false;
    out = _timeinfo;
    return true;
}
