#include "services/BrightnessService.h"
#include <Arduino.h>

#include "services/PreferencesService.h"

// Используем существующий PreferencesService
extern PreferencesService prefs;

// -----------------------------------------------------
// ctor
// -----------------------------------------------------
BrightnessService::BrightnessService() {
}

// -----------------------------------------------------
// begin
// -----------------------------------------------------
void BrightnessService::begin() {

    // читаем сохранённое значение
    _value = clamp(prefs.brightness());

    // защита от мусора / нуля
    if (_value < 10) {
        _value = 100;
        prefs.setBrightness(_value);
        prefs.save();
    }
}

// -----------------------------------------------------
// set
// -----------------------------------------------------
void BrightnessService::set(uint8_t value) {

    uint8_t v = clamp(value);
    if (v == _value)
        return;

    _value = v;

    prefs.setBrightness(_value);
    prefs.save();
}

// -----------------------------------------------------
// get
// -----------------------------------------------------
uint8_t BrightnessService::get() const {
    return _value;
}

// -----------------------------------------------------
// apply (ThemeBlend)
// -----------------------------------------------------
ThemeBlend BrightnessService::apply(const ThemeBlend& in) const {

    ThemeBlend out = in;

    // 🔒 ФОН НИКОГДА НЕ ТРОГАЕМ
    out.bg = in.bg;

    // 🔽 затемняем ТОЛЬКО foreground-цвета
    out.fg      = scale565(in.fg);
    out.muted   = scale565(in.muted);
    out.accent  = scale565(in.accent);
    out.warn    = scale565(in.warn);
    out.success = scale565(in.success);

    return out;
}

// -----------------------------------------------------
// helpers
// -----------------------------------------------------
uint8_t BrightnessService::clamp(uint8_t v) const {

    if (v < 10)  return 10;
    if (v > 100) return 100;
    return v;
}

uint16_t BrightnessService::scale565(uint16_t c) const {

    float k = _value / 100.0f;

    uint8_t r = ((c >> 11) & 0x1F) * k;
    uint8_t g = ((c >> 5)  & 0x3F) * k;
    uint8_t b = ( c        & 0x1F) * k;

    return (r << 11) | (g << 5) | b;
}