#pragma once

#include <Arduino.h>
#include <time.h>
#include "core/ServiceVersion.h"

/*
 * ============================================================
 * TimeService
 *
 * Управляет системным временем ESP32 (NTP).
 * ЯВЛЯЕТСЯ ИСТОЧНИКОМ СОБЫТИЙ ДЛЯ UI (через versioning)
 * ============================================================
 */
class TimeService {
public:
    enum Weekday {
        SUN = 0, MON, TUE, WED, THU, FRI, SAT
    };

    enum SyncState {
        NOT_STARTED,
        SYNCING,
        SYNCED,
        ERROR
    };

    TimeService();

    void setTimezone(int utcOffset, int dst);
    void begin();
    void update();

    // ===== VERSION =====
    const ServiceVersion& version() const;

    // ===== SYNC STATUS =====
    SyncState syncState() const;
    bool isValid() const;

    // ===== TIME =====
    int hour()   const;
    int minute() const;
    int second() const;

    // ===== DATE =====
    int day()    const;
    int month()  const;
    int year()   const;
    Weekday weekday() const;

private:
    void applyTimezone();

    tm _tm {};
    tm _prevTm {};

    bool _valid = false;
    bool _wasValid = false;

    unsigned long _lastUpdateMs = 0;
    static constexpr unsigned long UPDATE_INTERVAL = 1000;

    // timezone
    int _utcOffset = 0;
    int _dstOffset = 0;

    // sync state
    SyncState _syncState = NOT_STARTED;

    // 🔥 VERSION
    ServiceVersion _version;
};