#pragma once
#include <time.h>

/*
 * TimeProvider
 * ------------
 * Интерфейс асинхронного поставщика времени.
 *
 * ВАЖНО:
 *  - update() НИКОГДА не должен блокировать (никаких delay / ожиданий)
 *  - provider может вернуть время "когда будет готов" (RTC сразу, NTP позже)
 *  - TimeService агрегирует несколько providers и решает, кого слушать
 */

struct TimeResult {
    tm   time{};
    bool valid = false;
};

class TimeProvider {
public:
    virtual ~TimeProvider() = default;

    // Вызывается часто из loop() — НЕ блокировать!
    virtual void update() = 0;

    // Есть ли новое время, готовое к выдаче
    virtual bool hasTime() const = 0;

    // Забрать результат (обычно "одноразовый" — после takeTime() hasTime() → false)
    virtual TimeResult takeTime() = 0;
};