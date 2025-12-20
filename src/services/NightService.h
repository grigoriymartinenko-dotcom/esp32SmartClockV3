#pragma once
#include <stdint.h>

#include "services/UiVersionService.h"
#include "services/TimeService.h"

/*
 * NightService
 * ------------
 * Определяет ночь / день.
 * Логическое событие → THEME version
 */

class NightService {
public:
    // 🔹 НОВОЕ (v3.2)
    explicit NightService(UiVersionService& uiVersion);

    void begin();
    void update(const TimeService& time);

    bool isNight() const;

private:
    UiVersionService& _uiVersion;

    bool _isNight = false;
};