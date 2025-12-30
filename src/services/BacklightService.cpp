#include "services/BacklightService.h"
#include <Arduino.h>
#include "config/Pins.h"   // ← ВАЖНО
// ============================================================================
// PWM CONFIG (LEDC)
// ============================================================================
// Частота 5kHz и 8-bit разрешение — очень распространённые безопасные значения.
// 8-bit даёт 0..255 (достаточно плавно для подсветки).
static constexpr int PWM_CH   = 0;
static constexpr int PWM_FREQ = 5000; // Hz
static constexpr int PWM_RES  = 8;    // bits (0..255)

// Если на твоём модуле подсветка активна "наоборот" (инверсия):
//  - при duty=0 → светит ярко
//  - при duty=255 → гаснет
// тогда поставь true.
static constexpr bool BACKLIGHT_INVERTED = false;

// ============================================================================
// helpers
// ============================================================================
static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

// ============================================================================
// public API
// ============================================================================
void BacklightService::begin() {
    // TFT_BL берём из main.cpp (там он в pinout).
    pinMode(TFT_BL, OUTPUT);

    // Настраиваем PWM-канал и цепляем его к пину.
    // Для Arduino-ESP32 это стандартный API LEDC.
    ledcSetup(PWM_CH, PWM_FREQ, PWM_RES);
    ledcAttachPin(TFT_BL, PWM_CH);

    apply();
}

void BacklightService::set(float value) {
    _value = clamp01(value);
    apply();
}

float BacklightService::get() const {
    return _value;
}

// ============================================================================
// internals
// ============================================================================
void BacklightService::apply() {
    // value 0..1 → duty 0..255
    uint8_t duty = (uint8_t)(_value * 255.0f + 0.5f);

    if (BACKLIGHT_INVERTED) {
        duty = 255 - duty;
    }

    ledcWrite(PWM_CH, duty);
}