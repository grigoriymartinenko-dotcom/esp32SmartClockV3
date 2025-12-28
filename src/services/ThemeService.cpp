#include "services/ThemeService.h"
#include "theme/Themes.h"
#include <Arduino.h>

// ============================================================================
// ctor / init
// ============================================================================

ThemeService::ThemeService(UiVersionService& uiVersion)
    : _uiVersion(uiVersion)
{
}

void ThemeService::begin() {
    _theme = THEME_DAY;
    _night = false;
}

// ============================================================================
// logical theme state
// ============================================================================

void ThemeService::setNight(bool night) {
    if (_night == night)
        return;

    _night = night;
    _theme = _night ? THEME_NIGHT : THEME_DAY;

    // 🔥 ЛОГИЧЕСКОЕ событие смены темы
    // (не анимация, не переход, а факт)
    _uiVersion.bump(UiChannel::THEME);
}

bool ThemeService::isNight() const {
    return _night;
}

const Theme& ThemeService::current() const {
    return _theme;
}

// ============================================================================
// color helpers
// ============================================================================

uint16_t ThemeService::blend565(uint16_t day, uint16_t night, float k) {
    // Защита от выхода за диапазон
    if (k < 0.0f) k = 0.0f;
    if (k > 1.0f) k = 1.0f;

    // Распаковка RGB565 (5-6-5)
    uint8_t r1 = (day   >> 11) & 0x1F;
    uint8_t g1 = (day   >> 5 ) & 0x3F;
    uint8_t b1 =  day          & 0x1F;

    uint8_t r2 = (night >> 11) & 0x1F;
    uint8_t g2 = (night >> 5 ) & 0x3F;
    uint8_t b2 =  night        & 0x1F;

    // Линейная интерполяция
    uint8_t r = r1 + (r2 - r1) * k;
    uint8_t g = g1 + (g2 - g1) * k;
    uint8_t b = b1 + (b2 - b1) * k;

    // Обратная сборка в RGB565
    return (r << 11) | (g << 5) | b;
}