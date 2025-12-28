#pragma once

#include <Arduino.h>

/*
 * NightTransitionService
 * ----------------------
 * Плавный переход DAY <-> NIGHT.
 *
 * Отвечает ТОЛЬКО за коэффициент ночи:
 *   0.0 — день
 *   1.0 — ночь
 *
 * НЕ:
 *  - рисует
 *  - меняет цвета
 *  - знает про UI
 */

class NightTransitionService {
public:
    NightTransitionService();

    // ------------------------------------------------------------------------
    // control
    // ------------------------------------------------------------------------
    void setTarget(bool night);      // true = ночь, false = день
    void update();                   // вызывать в loop()

    // ------------------------------------------------------------------------
    // state
    // ------------------------------------------------------------------------
    bool  transitioning() const;
    float nightFactor() const;       // 0.0 .. 1.0

private:
    bool    _targetNight;
    float   _factor;
    uint32_t _lastMs;

    static constexpr float SPEED = 0.00025f; // скорость изменения в мс
};