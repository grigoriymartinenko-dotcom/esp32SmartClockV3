#pragma once
#include <time.h>

/*
 * TimeService
 * Гарантированно поднимает системное время через NTP
 */
class TimeService {
public:
    void begin();
    void update();

    bool isValid() const;

    int hour() const;
    int minute() const;
    int second() const;

private:
    bool _valid = false;
    tm _tm{};
};