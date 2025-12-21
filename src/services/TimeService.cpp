#include "services/TimeService.h"
#include <Arduino.h>
#include <sys/time.h>   // settimeofday

TimeService::TimeService(UiVersionService& uiVersion)
    : _uiVersion(uiVersion)
{
}

void TimeService::begin() {
    // NTP стартует, но НЕ блокирует
    syncNtp();
}

void TimeService::setTimezone(long gmtOffsetSec, int daylightOffsetSec) {
    _gmtOffsetSec = gmtOffsetSec;
    _daylightOffsetSec = daylightOffsetSec;

    // задаём NTP серверы для системного времени
    configTime(_gmtOffsetSec, _daylightOffsetSec, "pool.ntp.org");
}

void TimeService::setFromRtc(const tm& t) {
    _timeinfo = t;
    _valid = true;
    _source = RTC;

    _lastMinute = t.tm_min;
    _lastSecond = t.tm_sec;

    // ==================================================
    // 🔥 КЛЮЧЕВО:
    // выставляем системное время ESP32 из RTC,
    // чтобы getLocalTime() работал сразу, без ожидания NTP.
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
        // если времени ещё нет вообще — помечаем ошибку/ожидание
        if (!_valid) {
            // NTP ещё не пришёл, RTC мог не быть
            if (_syncState == SYNCING) {
                // остаёмся в SYNCING, это НЕ ошибка "навсегда"
                // но если хочешь — можно таймером перевести в ERROR
            } else {
                _syncState = ERROR;
            }
        }
        return;
    }

    // системное время есть → обновляем кэш
    _timeinfo = t;
    _valid = true;

    // если NTP ещё не помечен как SYNCED — считаем что он пришёл
    // (на практике getLocalTime начинает давать валидное время после SNTP)
    _source = NTP;
    _syncState = SYNCED;

    if (t.tm_min != _lastMinute) {
        _lastMinute = t.tm_min;
        _uiVersion.bump(UiChannel::TIME);
    }

    if (t.tm_sec != _lastSecond) {
        _lastSecond = t.tm_sec;
        _uiVersion.bump(UiChannel::TIME); // blink/seconds
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