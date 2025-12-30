#pragma once
#include "services/TimeProvider.h"
#include "services/RtcService.h"

/*
 * RtcTimeProvider
 * ---------------
 * Provider для RTC (DS1302).
 *
 * Поведение:
 *  - читает RTC ОДИН РАЗ (при первом update)
 *  - если время валидно — отдаёт TimeResult и "замолкает"
 *
 * Почему так:
 *  - RTC нужен, чтобы сразу после boot было "примерное" время
 *  - дальше время будет уточнено NTP (если AUTO) и тиканье пойдёт от системного времени
 */

class RtcTimeProvider : public TimeProvider {
public:
    explicit RtcTimeProvider(RtcService& rtc);

    void update() override;
    bool hasTime() const override;
    TimeResult takeTime() override;

private:
    RtcService& _rtc;

    bool _done = false;     // уже читали RTC
    bool _ready = false;    // есть результат
    tm   _tm{};             // сохранённое время
};