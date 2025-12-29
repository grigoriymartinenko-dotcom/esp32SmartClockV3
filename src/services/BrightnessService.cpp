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

    // Читаем из PreferencesData
    _value = clamp(prefs.brightness());

    // Если в EEPROM было 0 (или мусор) — ставим 100%
    if (_value == 0) {
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

    // сохраняем в PreferencesData
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
// apply
// -----------------------------------------------------
void BrightnessService::apply() {

    if (!_apply)
        return;

    // 0..100 → 0..255
    uint8_t hw = (uint8_t)((_value * 255) / 100);
    _apply(hw);
}

// -----------------------------------------------------
// attach
// -----------------------------------------------------
void BrightnessService::attach(ApplyFn fn) {
    _apply = fn;
}

// -----------------------------------------------------
// clamp
// -----------------------------------------------------
uint8_t BrightnessService::clamp(uint8_t v) const {

    if (v > 100)
        return 100;

    return v;
}