#include "services/TimeService.h"
#include <Arduino.h>
#include <sys/time.h>   // settimeofday

TimeService::TimeService(UiVersionService& uiVersion)
    : _uiVersion(uiVersion)
{
}

void TimeService::begin() {
    // Стартуем NTP, но не блокируем систему
    syncNtp();
}

void TimeService::setTimezone(long gmtOffsetSec, int daylightOffsetSec) {
    _gmtOffsetSec      = gmtOffsetSec;
    _daylightOffsetSec = daylightOffsetSec;

    // Первичная установка (DST пока 0 — пересчитается в update)
    configTime(_gmtOffsetSec, 0, "pool.ntp.org");
}

void TimeService::setFromRtc(const tm& t) {
    _timeinfo = t;
    _valid = true;
    _source = RTC;

    _lastMinute = t.tm_min;
    _lastSecond = t.tm_sec;

    // ==================================================
    // ВАЖНО:
    // выставляем system time ESP32,
    // чтобы getLocalTime() сразу работал
    // ==================================================
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

void TimeService::update() {
    updateTime();
}

void TimeService::updateTime() {
    tm t;
    if (!getLocalTime(&t)) {
        if (!_valid) {
            if (_syncState != SYNCING)
                _syncState = ERROR;
        }
        return;
    }

    // system time валидно
    _timeinfo = t;
    _valid = true;

    // ===== NTP confirmation =====
    if (_syncState == SYNCING && !_ntpConfirmed) {
        _ntpConfirmed = true;
        _source = NTP;
        _syncState = SYNCED;
    }

    // ===== DST AUTO =====
    bool newDst = _dst.isDst(t);

    if (newDst != _dstActive) {
        _dstActive = newDst;

        // Переустанавливаем timezone
        configTime(
            _gmtOffsetSec,
            _dstActive ? _daylightOffsetSec : 0,
            "pool.ntp.org"
        );

        // Обновляем UI (часы пересчитаются)
        _uiVersion.bump(UiChannel::TIME);
    }

    // ===== UI updates =====
    if (t.tm_min != _lastMinute) {
        _lastMinute = t.tm_min;
        _uiVersion.bump(UiChannel::TIME);
    }

    if (t.tm_sec != _lastSecond) {
        _lastSecond = t.tm_sec;
        _uiVersion.bump(UiChannel::TIME);
    }
}

void TimeService::syncNtp() {
    _syncState = SYNCING;
    _ntpConfirmed = false;
}

bool TimeService::isValid() const {
    return _valid;
}

int TimeService::hour()   const { return _timeinfo.tm_hour; }
int TimeService::minute() const { return _timeinfo.tm_min;  }
int TimeService::second() const { return _timeinfo.tm_sec;  }

int TimeService::day()   const { return _timeinfo.tm_mday; }
int TimeService::month() const { return _timeinfo.tm_mon + 1; }
int TimeService::year()  const { return _timeinfo.tm_year + 1900; }

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