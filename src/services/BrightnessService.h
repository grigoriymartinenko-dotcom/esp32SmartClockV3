#pragma once
#include <stdint.h>

#include "services/ThemeBlend.h"

/*
 * BrightnessService (Variant B)
 * -----------------------------
 * Псевдо-яркость UI через ThemeBlend.
 *
 * ВАЖНО:
 *  - НЕ управляет подсветкой
 *  - НЕ знает про TFT / PWM
 *  - НЕ трогает bg (фон)
 *  - Минимум = 10%
 */

class BrightnessService {
public:
    BrightnessService();

    void begin();

    void set(uint8_t value);   // 0..100
    uint8_t get() const;

    // Применение к ThemeBlend
    ThemeBlend apply(const ThemeBlend& in) const;

private:
    uint8_t clamp(uint8_t v) const;
    uint16_t scale565(uint16_t c) const;

private:
    uint8_t _value = 100;   // 10..100
};