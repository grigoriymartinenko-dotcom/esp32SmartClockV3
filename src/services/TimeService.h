#pragma once

#include <time.h>
#include <stdint.h>

#include "services/UiVersionService.h"
#include "services/DstService.h"

/*
 * TimeService
 * -----------
 * Единый источник времени для всей системы.
 *
 * Режимы:
 *  - RTC_ONLY   — использовать только RTC
 *  - NTP_ONLY   — использовать только NTP
 *  - LOCAL_ONLY — время не обновляется (ручное / system)
 *  - AUTO       — RTC → затем уточнение NTP
 */
class TimeService {
public:
    // ===== режим источника времени =====
    enum Mode {
        RTC_ONLY,
        NTP_ONLY,
        LOCAL_ONLY,
        AUTO
    };

    // ===== состояние синхронизации =====
    enum SyncState {
        NOT_STARTED,
        SYNCING,
        SYNCED,
        ERROR
    };

    // ===== текущий источник =====
    enum Source {
        NONE,
        RTC,
        NTP
    };

    explicit TimeService(UiVersionService& uiVersion);

    // ===== lifecycle =====
    void begin();
    void update();

    // ===== mode =====
    void setMode(Mode m);
    Mode mode() const;

    // ===== timezone / DST =====
    void setTimezone(long gmtOffsetSec, int daylightOffsetSec);

    // ===== RTC =====
    void setFromRtc(const tm& t);

    // ===== time access =====
    bool isValid() const;

    int hour()   const;
    int minute() const;
    int second() const;

    int day()   const;
    int month() const;
    int year()  const;

    SyncState syncState() const;
    Source    source()    const;

    // ===== export =====
    bool getTm(tm& out) const;

    bool isDstActive() const { return _dstActive; }

private:
    void updateTime();
    void syncNtp();

private:
    UiVersionService& _uiVersion;

    Mode      _mode       = AUTO;
    SyncState _syncState  = NOT_STARTED;
    Source    _source     = NONE;

    bool _ntpConfirmed = false;
    bool _valid        = false;

    tm _timeinfo{};

    int _lastMinute = -1;
    int _lastSecond = -1;

    long _gmtOffsetSec      = 0;
    int  _daylightOffsetSec = 0;

    DstService _dst;
    bool _dstActive = false;
};