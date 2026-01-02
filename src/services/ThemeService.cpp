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
    _cachedBlend = interpolate(0.0f);
}

// ============================================================================
// state
// ============================================================================

void ThemeService::setNight(bool night) {
    if (_night == night) return;

    _night = night;
    _theme = _night ? THEME_NIGHT : THEME_DAY;

    _uiVersion.bump(UiChannel::THEME);
}

bool ThemeService::isNight() const {
    return _night;
}

const Theme& ThemeService::current() const {
    return _theme;
}

// ============================================================================
// helpers (PUBLIC API — НЕ УДАЛЯТЬ)
// ============================================================================

uint16_t ThemeService::blend565(uint16_t a, uint16_t b, float k) {
    uint8_t ar = (a >> 11) & 0x1F;
    uint8_t ag = (a >> 5)  & 0x3F;
    uint8_t ab =  a        & 0x1F;

    uint8_t br = (b >> 11) & 0x1F;
    uint8_t bg = (b >> 5)  & 0x3F;
    uint8_t bb =  b        & 0x1F;

    uint8_t r = ar + (br - ar) * k;
    uint8_t g = ag + (bg - ag) * k;
    uint8_t b2 = ab + (bb - ab) * k;

    return (r << 11) | (g << 5) | b2;
}

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) |
           ((g & 0xFC) << 3) |
           ( b >> 3);
}

// ============================================================================
// compat: старые экраны
// ============================================================================
// ============================================================================
// blend() — КЕШИРОВАННЫЙ (СТАРЫЙ API)
// ============================================================================

const ThemeBlend& ThemeService::blend() const {
    // k = 0 для day, 1 для night
    float k = _night ? 1.0f : 0.0f;
    _cachedBlend = interpolate(k);
    return _cachedBlend;
}
// ============================================================================
// interpolate → ThemeBlend (УСИЛЕННЫЙ КОНТРАСТ)
// ============================================================================

ThemeBlend ThemeService::interpolate(float k) const {

    ThemeBlend out{};

    // background / foreground
    out.bg = blend565(THEME_DAY.bg, THEME_NIGHT.bg, k);
    out.fg = blend565(THEME_DAY.textPrimary, THEME_NIGHT.textPrimary, k);

    // muted — вторичный, тёмный
    out.muted = blend565(
        rgb565(140, 140, 140),
        rgb565(90,  90,  90),
        k
    );

    // accent — из темы
    out.accent = blend565(THEME_DAY.accent, THEME_NIGHT.accent, k);

    // success — ЯВНО зелёный
    out.success = blend565(
        rgb565(0, 200, 0),
        rgb565(0, 120, 0),
        k
    );

    // warn / error
    out.warn = blend565(THEME_DAY.error, THEME_NIGHT.error, k);

    return out;
}