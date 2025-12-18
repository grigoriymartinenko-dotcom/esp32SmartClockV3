#pragma once
#include <Arduino.h>

/*
 * TimeService
 * ===========
 * Источник текущего времени (пока — заглушка)
 * Позже подключим RTC + NTP
 */
class TimeService {
public:
    void begin();
    void update();

    uint8_t hour() const;
    uint8_t minute() const;
    uint8_t second() const;

private:
    unsigned long _lastMs = 0;
    uint8_t _h = 12;
    uint8_t _m = 0;
    uint8_t _s = 0;
};