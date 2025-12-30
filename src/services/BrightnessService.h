#pragma once
#include <stdint.h>
#include "services/ThemeBlend.h"

/*
 * BrightnessService
 * -----------------
 * ЕДИНЫЙ источник истины по яркости UI.
 *
 * Отвечает за:
 *  - хранение текущей яркости (0.1 .. 1.0)
 *  - загрузку / сохранение prefs
 *  - применение яркости к ThemeBlend
 *
 * НЕ управляет подсветкой TFT (это отдельный шаг).
 */

class BrightnessService {
public:
    void begin();

    // value: 0.1f .. 1.0f
    void set(float value);
    float get() const;

    // применить текущую яркость к ThemeBlend
    ThemeBlend apply(const ThemeBlend& in) const;

private:
    float _value = 1.0f;

    static uint16_t scale(uint16_t rgb565, float k);
};