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
    _rtcWritten   = false;   // 🔴 ВАЖНО

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
    setSource(RTC);

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

    // ===== SOURCE LOGIC (ЕДИНСТВЕННОЕ МЕСТО) =====
    switch (_mode) {
        case RTC_ONLY:
            _source = RTC;
            break;

        case NTP_ONLY:
            _source = NTP;
            break;

        case LOCAL_ONLY:
            _source = NONE;
            break;

        case AUTO:
            _source = _ntpConfirmed ? NTP : RTC;
            break;
    }

    // ===== подтверждение NTP (ТОЛЬКО ДЛЯ AUTO / NTP_ONLY) =====
    if ((_mode == AUTO || _mode == NTP_ONLY) &&
        _syncState == SYNCING && !_ntpConfirmed) {

        _ntpConfirmed = true;
        _syncState    = SYNCED;
    }

    // ===== DST =====
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

    // ===== UI bump =====
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
// SOURCE (ЕДИНСТВЕННАЯ ТОЧКА)
// ------------------------------------------------------------
void TimeService::setSource(Source s) {
    if (_source == s)
        return;

    _source = s;

    // 🔥 КЛЮЧЕВОЕ СОБЫТИЕ:
    // UI обязан узнать, что источник сменился
    _uiVersion.bump(UiChannel::TIME);
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

bool TimeService::shouldWriteRtc() const {
    // RTC обновляем ТОЛЬКО в AUTO
    if (_mode != AUTO)
        return false;

    // только после подтверждённого NTP
    if (_syncState != SYNCED)
        return false;

    // только один раз
    if (_rtcWritten)
        return false;

    return true;
}

void TimeService::markRtcWritten() {
    _rtcWritten = true;
}