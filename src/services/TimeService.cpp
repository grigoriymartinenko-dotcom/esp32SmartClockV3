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
// registerProvider
// ------------------------------------------------------------
void TimeService::registerProvider(TimeProvider& p) {
    if (_providersCount >= MAX_PROVIDERS) return;
    _providers[_providersCount++] = &p;
}

// ------------------------------------------------------------
// begin
// ------------------------------------------------------------
void TimeService::begin() {

    _syncState      = NOT_STARTED;
    _ntpConfirmed   = false;
    _rtcWritten     = false;

    _rtcAppliedOnce = false;
    _ntpAppliedOnce = false;

    _valid          = false;
    _source         = NONE;

    _lastMinute     = -1;
    _lastSecond     = -1;

    if (_mode == AUTO || _mode == NTP_ONLY) {
        syncNtp();
    } else {
        _syncState = NOT_STARTED;
    }

    _uiVersion.bump(UiChannel::TIME);
}

// ------------------------------------------------------------
// update
// ------------------------------------------------------------
void TimeService::update() {

    if (_mode == LOCAL_ONLY)
        return;

    // 1) provider stage (RTC first, NTP second)
    tryConsumeProviders();

    // 2) system clock tick (seconds should advance here)
    updateFromSystemClock();
}

// ------------------------------------------------------------
// MODE
// ------------------------------------------------------------
void TimeService::setMode(Mode m) {
    if (_mode == m)
        return;

    _mode          = m;
    _syncState     = NOT_STARTED;
    _ntpConfirmed  = false;
    _rtcWritten    = false;

    _rtcAppliedOnce = false;
    _ntpAppliedOnce = false;

    if (_mode == AUTO || _mode == NTP_ONLY) {
        syncNtp();
    }

    if (_mode == LOCAL_ONLY) {
        setSource(NONE);
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

    // Базовый TZ. DST включаем/выключаем динамически в updateFromSystemClock().
    configTime(_gmtOffsetSec, 0, "pool.ntp.org");
}

// ------------------------------------------------------------
// RTC inject (compat)
// ------------------------------------------------------------
void TimeService::setFromRtc(const tm& t) {
    if (_mode == NTP_ONLY || _mode == LOCAL_ONLY)
        return;

    if (!looksValid(t))
        return;

    // ВАЖНО: применяем RTC только один раз,
    // иначе можно "прибить" системные секунды.
    if (_rtcAppliedOnce)
        return;

    _timeinfo = t;
    _valid    = true;

    applySystemTime(t);
    _rtcAppliedOnce = true;

    setSource(RTC);

    _uiVersion.bump(UiChannel::TIME);
}

// ------------------------------------------------------------
// helpers
// ------------------------------------------------------------
bool TimeService::looksValid(const tm& t) {
    const int y = t.tm_year + 1900;
    return (y >= 2020 && y <= 2099);
}

// ------------------------------------------------------------
// applySystemTime
// ------------------------------------------------------------
void TimeService::applySystemTime(const tm& t) {

    tm tmp = t;

    // mktime() нормализует tm_wday/tm_yday (важно для weekday в UI)
    time_t epoch = mktime(&tmp);
    if (epoch <= 0) return;

    timeval tv{};
    tv.tv_sec  = epoch;
    tv.tv_usec = 0;

    settimeofday(&tv, nullptr);
}

// ------------------------------------------------------------
// providers
// ------------------------------------------------------------
void TimeService::tryConsumeProviders() {

    // ВАЖНО: порядок providers = приоритет.
    // Ожидаем:
    //   [0] RTC provider
    //   [1] NTP provider
    //
    // Это позволяет 100% отличать "кто дал время" без RTTI и без type field.
    for (uint8_t i = 0; i < _providersCount; i++) {

        TimeProvider* p = _providers[i];
        if (!p) continue;

        p->update();
        if (!p->hasTime()) continue;

        TimeResult r = p->takeTime();
        if (!r.valid) continue;

        if (!looksValid(r.time))
            continue;

        const bool isRtcProvider = (i == 0);
        const bool isNtpProvider = (i == 1);

        // --------------------------------------------------------
        // MODE FILTERS
        // --------------------------------------------------------
        if (_mode == RTC_ONLY) {
            if (!isRtcProvider) continue;
            if (_rtcAppliedOnce) continue;
        }

        if (_mode == NTP_ONLY) {
            if (!isNtpProvider) continue;
            if (_ntpAppliedOnce) continue;
        }

        if (_mode == AUTO) {
            // RTC можно один раз, NTP можно один раз.
            if (isRtcProvider && _rtcAppliedOnce) continue;
            if (isNtpProvider && _ntpAppliedOnce) continue;
        }

        // --------------------------------------------------------
        // APPLY ONCE POLICY (самый важный фикс)
        // --------------------------------------------------------
        _timeinfo = r.time;
        _valid    = true;

        applySystemTime(r.time);

        if (isRtcProvider) {
            _rtcAppliedOnce = true;

            if (_mode != NTP_ONLY) {
                setSource(RTC);
            }

            // В AUTO мы могли быть в SYNCING (ждём NTP),
            // но RTC сам по себе SYNCED не делает.
        }

        if (isNtpProvider) {
            _ntpAppliedOnce = true;

            if (_mode == AUTO || _mode == NTP_ONLY) {
                _ntpConfirmed = true;
                _syncState    = SYNCED;
                setSource(NTP);
            }
        }

        _uiVersion.bump(UiChannel::TIME);
        break;
    }
}

// ------------------------------------------------------------
// system clock tick + DST + UX timeout
// ------------------------------------------------------------
void TimeService::updateFromSystemClock() {

    // UX: если долго SYNCING — считаем ошибкой
    if (_syncState == SYNCING && millis() - _syncStartedAt > 15000) {
        _syncState = ERROR;
        _uiVersion.bump(UiChannel::TIME);
    }

    tm t{};
    if (!getLocalTime(&t)) {
        return;
    }

    // нормализуем wday/yday
    mktime(&t);

    _timeinfo = t;
    _valid    = true;

    // source logic (совместимость)
    switch (_mode) {
        case RTC_ONLY:   setSource(RTC); break;
        case NTP_ONLY:   setSource(NTP); break;
        case LOCAL_ONLY: setSource(NONE); break;
        case AUTO:       setSource(_ntpConfirmed ? NTP : RTC); break;
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

    // tick (это и должно заставлять секунды "идти")
    if (t.tm_min != _lastMinute || t.tm_sec != _lastSecond) {
        _lastMinute = t.tm_min;
        _lastSecond = t.tm_sec;
        _uiVersion.bump(UiChannel::TIME);
    }
}

// ------------------------------------------------------------
// NTP UX
// ------------------------------------------------------------
void TimeService::syncNtp() {
    _syncState = SYNCING;
    _syncStartedAt = millis();
    _uiVersion.bump(UiChannel::TIME);
}

// ------------------------------------------------------------
// source
// ------------------------------------------------------------
void TimeService::setSource(Source s) {
    if (_source == s) return;
    _source = s;
    _uiVersion.bump(UiChannel::TIME);
}

// ------------------------------------------------------------
// getters
// ------------------------------------------------------------
bool TimeService::isValid() const { return _valid; }

int TimeService::hour()   const { return _timeinfo.tm_hour; }
int TimeService::minute() const { return _timeinfo.tm_min;  }
int TimeService::second() const { return _timeinfo.tm_sec;  }

int TimeService::day()   const { return _timeinfo.tm_mday; }
int TimeService::month() const { return _timeinfo.tm_mon + 1; }
int TimeService::year()  const { return _timeinfo.tm_year + 1900; }

TimeService::SyncState TimeService::syncState() const { return _syncState; }
TimeService::Source    TimeService::source()    const { return _source; }

bool TimeService::getTm(tm& out) const {
    if (!_valid) return false;
    out = _timeinfo;
    return true;
}

// ------------------------------------------------------------
// RTC write policy
// ------------------------------------------------------------
bool TimeService::shouldWriteRtc() const {
    return _mode == AUTO && _syncState == SYNCED && !_rtcWritten;
}

void TimeService::markRtcWritten() {
    _rtcWritten = true;
}

// ------------------------------------------------------------
// UX helpers (labels only)
// ------------------------------------------------------------
const char* TimeService::sourceLabel() const {
    switch (_source) {
        case RTC:  return "RTC";
        case NTP:  return "NTP";
        case NONE: return "LOCAL";
        default:   return "---";
    }
}

const char* TimeService::stateLabel() const {
    if (_mode == LOCAL_ONLY)
        return "disabled";

    switch (_syncState) {
        case NOT_STARTED: return "idle";
        case SYNCING:     return "syncing";
        case SYNCED:      return "ok";
        case ERROR:       return "error";
        default:          return "---";
    }
}