#pragma once
#include <time.h>
#include <stdint.h>

#include "services/UiVersionService.h"
#include "services/DstService.h"

/*
 * TimeService
 * -----------
 * Источник времени:
 *  - RTC → первичный (при старте)
 *  - NTP → вторичный (уточнение)
 *
 * ОСНОВНАЯ ИДЕЯ:
 *  - ESP32 всегда живёт в system time
 *  - getLocalTime() используется ВЕЗДЕ
 *  - DST применяется ДИНАМИЧЕСКИ
 */
class TimeService {
public:
    enum SyncState {
        NOT_STARTED,
        SYNCING,
        SYNCED,
        ERROR
    };

    enum Source {
        NONE,
        RTC,
        NTP
    };

    explicit TimeService(UiVersionService& uiVersion);

    void begin();
    void update();

    // Базовый timezone:
    // gmtOffsetSec = UTC+X
    // daylightOffsetSec = +3600 (DST)
    void setTimezone(long gmtOffsetSec, int daylightOffsetSec);

    // ===== RTC =====
    void setFromRtc(const tm& t);

    // ===== TIME =====
    int hour()   const;
    int minute() const;
    int second() const;

    // ===== DATE =====
    bool isValid() const;

    int day()   const;
    int month() const;
    int year()  const;

    SyncState syncState() const;
    Source source() const;

    // экспорт текущего времени (для RTC)
    bool getTm(tm& out) const;
bool isDstActive() const { return _dstActive; }
private:
    void updateTime();
    void syncNtp();

private:
    UiVersionService& _uiVersion;

    tm   _timeinfo{};
    bool _valid = false;

    Source    _source    = NONE;
    SyncState _syncState = NOT_STARTED;

    bool _ntpConfirmed = false;

    int _lastMinute = -1;
    int _lastSecond = -1;

    // ===== Timezone / DST =====
    long _gmtOffsetSec = 0;
    int  _daylightOffsetSec = 0;

    DstService _dst;
    bool _dstActive = false;
};