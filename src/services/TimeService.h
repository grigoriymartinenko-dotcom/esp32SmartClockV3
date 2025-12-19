#pragma once

#include <Arduino.h>
#include <time.h>

/*
 * ============================================================
 * TimeService
 *
 * Отвечает ТОЛЬКО за системное время ESP32.
 * NTP настраивается в begin().
 * ============================================================
 */
class TimeService {
public:
    TimeService();

    void begin();   // настраивает NTP
    void update();  // обновляет кеш времени

    bool isValid() const;

    // время
    int hour()   const;
    int minute() const;
    int second() const;

    // дата
    int day()    const;
    int month()  const;
    int year()   const;

    // день недели (0 = воскресенье)
    int weekday() const;

private:
    tm _tm {};
    bool _valid = false;

    unsigned long _lastUpdateMs = 0;
    static constexpr unsigned long UPDATE_INTERVAL = 1000;
};