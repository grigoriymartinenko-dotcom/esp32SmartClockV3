#pragma once
#include "services/TimeProvider.h"

/*
 * NtpTimeProvider
 * ---------------
 * Асинхронный provider системного времени (SNTP/FreeRTOS time).
 *
 * Важное уточнение:
 *  - В Arduino-ESP32 NTP реально "живёт" внутри системы после configTime(...)
 *  - provider НЕ должен сам блокировать, он только:
 *      1) ждёт Wi-Fi (опционально)
 *      2) проверяет "стало ли системное время валидным"
 *      3) когда стало валидным — выдаёт его один раз
 *
 * Мы не делаем здесь configTime — это ответственность TimeService (timezone/DST).
 * Provider лишь детектит факт, что NTP синхронизация произошла.
 */

class NtpTimeProvider : public TimeProvider {
public:
    NtpTimeProvider();

    void update() override;
    bool hasTime() const override;
    TimeResult takeTime() override;

    // Если хочется: разрешить/запретить ожидание Wi-Fi (по умолчанию ждём Wi-Fi)
    void setRequireWifi(bool require) { _requireWifi = require; }

private:
    bool _ready = false;
    tm   _tm{};

    bool _requireWifi = true;

    bool systemTimeLooksValid(const tm& t) const;
};