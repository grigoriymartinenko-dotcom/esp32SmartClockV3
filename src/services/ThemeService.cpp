#include "services/ThemeService.h"

#include <Arduino.h>
#include <math.h>

#include "theme/Themes.h"

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
    // begin() — это старт приложения. Обычно bump не нужен.
}

// ============================================================================
// state
// ============================================================================

void ThemeService::setNight(bool night) {
    if (_night == night) return;

    _night = night;
    _theme = _night ? THEME_NIGHT : THEME_DAY;

    // Логическое событие смены базовой темы.
    // Экраны/виджеты могут подписываться на UiChannel::THEME.
    _uiVersion.bump(UiChannel::THEME);
}

bool ThemeService::isNight() const {
    return _night;
}

const Theme& ThemeService::current() const {
    return _theme;
}

// ============================================================================
// blending helpers
// ============================================================================

uint16_t ThemeService::blend565(uint16_t day, uint16_t night, float k) {
    // Clamp
    if (k < 0.0f) k = 0.0f;
    if (k > 1.0f) k = 1.0f;

    // Unpack RGB565 (5-6-5)
    const int r1 = (day   >> 11) & 0x1F;
    const int g1 = (day   >> 5 ) & 0x3F;
    const int b1 =  day          & 0x1F;

    const int r2 = (night >> 11) & 0x1F;
    const int g2 = (night >> 5 ) & 0x3F;
    const int b2 =  night        & 0x1F;

    // Linear interpolation in integer domain (avoid unsigned underflow!)
    int r = r1 + (int)lroundf((float)(r2 - r1) * k);
    int g = g1 + (int)lroundf((float)(g2 - g1) * k);
    int b = b1 + (int)lroundf((float)(b2 - b1) * k);

    // Clamp to channel sizes
    if (r < 0) r = 0; if (r > 0x1F) r = 0x1F;
    if (g < 0) g = 0; if (g > 0x3F) g = 0x3F;
    if (b < 0) b = 0; if (b > 0x1F) b = 0x1F;

    return (uint16_t)((r << 11) | (g << 5) | b);
}

Theme ThemeService::blended(float k) const {
    Theme out;

    out.bg            = blend565(THEME_DAY.bg,            THEME_NIGHT.bg,            k);
    out.textPrimary   = blend565(THEME_DAY.textPrimary,   THEME_NIGHT.textPrimary,   k);
    out.textSecondary = blend565(THEME_DAY.textSecondary, THEME_NIGHT.textSecondary, k);
    out.muted         = blend565(THEME_DAY.muted,         THEME_NIGHT.muted,         k);

    out.select        = blend565(THEME_DAY.select,        THEME_NIGHT.select,        k);
    out.warn          = blend565(THEME_DAY.warn,          THEME_NIGHT.warn,          k);
    out.accent        = blend565(THEME_DAY.accent,        THEME_NIGHT.accent,        k);
    out.error         = blend565(THEME_DAY.error,         THEME_NIGHT.error,         k);

    return out;
}