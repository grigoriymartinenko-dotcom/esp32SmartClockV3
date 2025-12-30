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

    _syncState    = NOT_STARTED;
    _ntpConfirmed = false;
    _rtcWritten   = false;

    // В AUTO / NTP_ONLY мы показываем UX "SYNCING" сразу.
    // Реальная синхронизация в Arduino-ESP32 начинается после configTime(...)
    // (который вызывается в setTimezone), а provider просто ждёт, когда время станет валидным.
    if (_mode == AUTO || _mode == NTP_ONLY) {
        syncNtp();
    }
}

// ------------------------------------------------------------
// update
// ------------------------------------------------------------
void TimeService::update() {

    if (_mode == LOCAL_ONLY) {
        return;
    }

    // 1) Сначала даём шанс провайдерам отдать новое время (RTC сразу, NTP позже)
    tryConsumeProviders();

    // 2) Затем обновляем "системное" время (тик каждую секунду, DST и UI bump)
    updateFromSystemClock();
}

// ------------------------------------------------------------
// MODE
// ------------------------------------------------------------
void TimeService::setMode(Mode m) {
    if (_mode == m)
        return;

    _mode         = m;
    _syncState    = NOT_STARTED;
    _ntpConfirmed = false;
    _rtcWritten   = false;

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

    // ВАЖНО:
    // - Здесь централизованно задаём timezone/DST для системного времени.
    // - NtpTimeProvider будет "детектить" готовность времени через getLocalTime().
    configTime(_gmtOffsetSec, 0, "pool.ntp.org");
}

// ------------------------------------------------------------
// RTC (compat)
// ------------------------------------------------------------
void TimeService::setFromRtc(const tm& t) {
    if (_mode == NTP_ONLY || _mode == LOCAL_ONLY)
        return;

    _timeinfo = t;
    _valid    = true;

    setSource(RTC);
    applySystemTime(t);
}

// ------------------------------------------------------------
// applySystemTime
// ------------------------------------------------------------
void TimeService::applySystemTime(const tm& t) {

    tm tmp = t;
    time_t epoch = mktime(&tmp);
    if (epoch <= 0) {
        return;
    }

    timeval tv{};
    tv.tv_sec  = epoch;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);
}

// ------------------------------------------------------------
// providers consumption
// ------------------------------------------------------------
void TimeService::tryConsumeProviders() {

    // Важно: порядок providers = приоритет.
    // AUTO: сначала скорее всего сработает RTC provider,
    //       позже NTP provider уточнит время.
    for (uint8_t i = 0; i < _providersCount; i++) {

        TimeProvider* p = _providers[i];
        if (!p) continue;

        p->update();

        if (!p->hasTime()) continue;

        TimeResult r = p->takeTime();
        if (!r.valid) continue;

        // Применяем режимы:
        // - RTC_ONLY: принимаем только RTC (и не ждём NTP)
        // - NTP_ONLY: принимаем только NTP
        // - AUTO: принимаем RTC первым, потом NTP (переключение RTC→NTP)
        //
        // Здесь мы не знаем "кто" provider по типу, поэтому определяем по логике:
        //   если уже NTP confirmed → повторные "NTP" не нужны,
        //   но допускаем один переход RTC→NTP в AUTO.
        //
        // Чтобы сохранить поведение UI (source), мы делаем так:
        //   - если _source == NONE или RTC → и пришло валидное время → считаем RTC этапом
        //   - если режим NTP_ONLY или AUTO и время стало валидным после SYNCING → считаем NTP
        //
        // Практически: RTC provider отдаёт очень рано (до валидного системного NTP),
        // а NTP provider отдаёт позже, когда getLocalTime уже даёт год >= 2020.
        bool accept = true;

        if (_mode == RTC_ONLY) {
            // В RTC_ONLY принимаем первое время и всё (не переходим на NTP)
            if (_source == NTP) accept = false;
        } else if (_mode == NTP_ONLY) {
            // В NTP_ONLY не хотим принимать "раннее RTC" вообще
            // (но setFromRtc может всё равно поставить время вручную — это отдельный путь)
            // Поэтому если мы ещё не SYNCED, то ждём NTP.
            // RTC provider в этом режиме просто не регистрируем в main.cpp.
            accept = true;
        } else if (_mode == AUTO) {
            // AUTO: разрешаем один переход RTC → NTP.
            // Если уже NTP confirmed — повторно не дергаем.
            accept = true;
        }

        if (!accept) continue;

        // Ставим время в систему
        _timeinfo = r.time;
        _valid = true;
        applySystemTime(r.time);

        // Определяем источник/статус синхронизации
        // 1) Если мы были в SYNCING и пришло "взрослое" системное время → это NTP подтверждение
        //    (обычно это будет NtpTimeProvider)
        // 2) Иначе — считаем RTC
        //
        // Важно: для сохранения твоего UX "R → R>" / "NTP syncing"
        // мы переводим в SYNCED только когда был SYNCING.
        if ((_mode == AUTO || _mode == NTP_ONLY) && _syncState == SYNCING && !_ntpConfirmed) {
            _ntpConfirmed = true;
            _syncState    = SYNCED;
            setSource(NTP);
        } else {
            // Раннее время (обычно RTC)
            if (_mode != NTP_ONLY) {
                setSource(RTC);
            }
        }

        // В AUTO если NTP уже подтвердился — _source станет NTP.
        // В RTC_ONLY — остаёмся RTC.
        // В NTP_ONLY — обычно будет NTP.

        // Любое событие получения времени — UI должен обновиться
        _uiVersion.bump(UiChannel::TIME);

        // Важно: в одном update() достаточно принять один источник
        // (RTC или NTP). Следующие будут приняты на следующих циклах.
        break;
    }
}

// ------------------------------------------------------------
// updateFromSystemClock
// ------------------------------------------------------------
void TimeService::updateFromSystemClock() {

    // В LOCAL_ONLY мы не обновляем время.
    if (_mode == LOCAL_ONLY)
        return;

    tm t{};
    if (!getLocalTime(&t)) {
        // Если режим NTP_ONLY и время так и не появилось — ошибка UX
        if (_mode == NTP_ONLY) {
            _syncState = ERROR;
            _uiVersion.bump(UiChannel::TIME);
        }
        return;
    }

    _timeinfo = t;
    _valid    = true;

    // ===== SOURCE LOGIC (совместимость с твоим старым API) =====
    switch (_mode) {
        case RTC_ONLY:
            setSource(RTC);
            break;

        case NTP_ONLY:
            setSource(NTP);
            break;

        case LOCAL_ONLY:
            setSource(NONE);
            break;

        case AUTO:
            setSource(_ntpConfirmed ? NTP : RTC);
            break;
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
// NTP (UX state)
// ------------------------------------------------------------
void TimeService::syncNtp() {
    _syncState = SYNCING;
    _uiVersion.bump(UiChannel::TIME); // UX: R → R>
}

// ------------------------------------------------------------
// SOURCE (ЕДИНСТВЕННАЯ ТОЧКА)
// ------------------------------------------------------------
void TimeService::setSource(Source s) {
    if (_source == s)
        return;

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

// ------------------------------------------------------------
// RTC write policy
// ------------------------------------------------------------
bool TimeService::shouldWriteRtc() const {
    if (_mode != AUTO)
        return false;
    if (_syncState != SYNCED)
        return false;
    if (_rtcWritten)
        return false;
    return true;
}

void TimeService::markRtcWritten() {
    _rtcWritten = true;
}