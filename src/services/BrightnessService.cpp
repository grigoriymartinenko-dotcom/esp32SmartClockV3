#include "services/BrightnessService.h"

// ------------------------------------------------------------
// helpers
// ------------------------------------------------------------
static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

// RGB565 → scale → RGB565
uint16_t BrightnessService::scale(uint16_t c, float k) {
    k = clamp01(k);

    uint8_t r = (c >> 11) & 0x1F;
    uint8_t g = (c >> 5)  & 0x3F;
    uint8_t b =  c        & 0x1F;

    r = uint8_t(r * k);
    g = uint8_t(g * k);
    b = uint8_t(b * k);

    return (r << 11) | (g << 5) | b;
}

// ------------------------------------------------------------
// public API
// ------------------------------------------------------------
void BrightnessService::begin() {
    // позже: загрузка из PreferencesService
    _value = 1.0f;
}

void BrightnessService::set(float value) {
    _value = clamp01(value);
}

float BrightnessService::get() const {
    return _value;
}

ThemeBlend BrightnessService::apply(const ThemeBlend& in) const {
    ThemeBlend out;
    out.bg       = scale(in.bg,       _value);
    out.fg       = scale(in.fg,       _value);
    out.accent   = scale(in.accent,   _value);
    out.muted    = scale(in.muted,    _value);
    out.warn     = scale(in.warn,     _value);
    out.success  = scale(in.success,  _value);
    return out;
}