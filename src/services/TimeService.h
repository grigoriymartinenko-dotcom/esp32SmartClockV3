#pragma once
#include <time.h>
#include <stdint.h>

#include "services/UiVersionService.h"

/*
 * TimeService
 * -----------
 * Источник времени + NTP
 * v3.2: версии
 * v3.1: старый API сохранён
 */

class TimeService {
public:
    enum SyncState {
        NOT_STARTED,
        SYNCING,
        SYNCED,
        ERROR
    };

    explicit TimeService(UiVersionService& uiVersion);

    void begin();
    void update();

    void setTimezone(long gmtOffsetSec, int daylightOffsetSec);

    // ===== TIME =====
    int hour()   const;
    int minute() const;
    int second() const;

    // ===== DATE (для StatusBar) =====
    bool isValid() const;

    int day()   const;
    int month() const;
    int year()  const;

    SyncState syncState() const;

private:
    void updateTime();
    void syncNtp();

private:
    UiVersionService& _uiVersion;

    tm _timeinfo{};
    SyncState _syncState = NOT_STARTED;

    int _lastMinute = -1;

    long _gmtOffsetSec = 0;
    int  _daylightOffsetSec = 0;
};