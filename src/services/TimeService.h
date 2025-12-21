#pragma once
#include <time.h>
#include <stdint.h>

#include "services/UiVersionService.h"

/*
 * TimeService
 * -----------
 * Источник времени:
 *  RTC → первичный
 *  NTP → вторичный (апдейт)
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
private:
    void updateTime();
    void syncNtp();

private:
    UiVersionService& _uiVersion;

    tm _timeinfo{};
    bool _valid = false;
    Source _source = NONE;

    SyncState _syncState = NOT_STARTED;

    int _lastMinute = -1;
    int _lastSecond = -1;

    long _gmtOffsetSec = 0;
    int  _daylightOffsetSec = 0;
};