#include "ui/StatusBar.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// ctor
// ============================================================================

StatusBar::StatusBar(
    Adafruit_ST7735& tft,
    ThemeService& theme,
    NightTransitionService& nightTransition,
    TimeService& time,
    WifiService& wifi
)
: _tft(tft)
, _theme(theme)
, _night(nightTransition)
, _time(time)
, _wifi(wifi)
{}

// ============================================================================
// public API
// ============================================================================

void StatusBar::markDirty() {
    _dirty = true;
}

// ---------------------------------------------------------------------------

void StatusBar::update() {

    Status newWifi = mapWifiStatus();
    Status newTime = mapTimeStatus();

    const Theme& th = _theme.current();

    // Фон тоже может плавно меняться → учитываем
    uint16_t blendedBg = ThemeService::blend565(
        THEME_DAY.bg,
        THEME_NIGHT.bg,
        _night.value()
    );

    if (newWifi != _wifiSt ||
        newTime != _timeSt ||
        blendedBg != _lastBg)
    {
        _wifiSt = newWifi;
        _timeSt = newTime;
        _lastBg = blendedBg;
        _dirty = true;
    }

    if (!_dirty) return;
    _dirty = false;

    drawStatic();
}

// ============================================================================
// static layer (редко)
// ============================================================================

void StatusBar::drawStatic() {

    const Theme& day   = THEME_DAY;
    const Theme& night = THEME_NIGHT;

    float k = _night.value();

    uint16_t bg = ThemeService::blend565(day.bg, night.bg, k);

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextWrap(false);

    // --- фон ---
    _tft.fillRect(0, 0, _tft.width(), HEIGHT, bg);

    const int Y1 = 4;
    const int Y2 = 14;
    const int DOT_X = 4;

    // --- точки статуса ---
    drawDot(DOT_X, Y1 + 4, statusDotColor(_wifiSt));
    drawDot(DOT_X, Y2 + 4, statusDotColor(_timeSt));

    // --- подписи ---
    uint16_t muted = ThemeService::blend565(day.muted, night.muted, k);
    _tft.setTextColor(muted, bg);

    _tft.setCursor(10, Y1);
    _tft.print("WiFi");

    _tft.setCursor(10, Y2);
    _tft.print(
        (_time.source() == TimeService::NTP) ? "NTP" :
        (_time.source() == TimeService::RTC) ? "RTC" : "---"
    );

    // сбрасываем кэш даты
    _lastTimeStr[0] = '\0';
}

// ============================================================================
// dynamic layer (TIME only)
// ============================================================================

void StatusBar::drawTimeOnly() {

    if (!_time.isValid()) return;

    tm t{};
    if (!_time.getTm(t)) return;

    char buf[32];
    snprintf(
        buf,
        sizeof(buf),
        "%s  %02d.%02d.%04d",
        weekdayEnFromTm(t),
        t.tm_mday,
        t.tm_mon + 1,
        t.tm_year + 1900
    );

    if (strcmp(buf, _lastTimeStr) == 0)
        return;

    strcpy(_lastTimeStr, buf);

    const Theme& day   = THEME_DAY;
    const Theme& night = THEME_NIGHT;
    float k = _night.value();

    uint16_t bg    = ThemeService::blend565(day.bg,    night.bg,    k);
    uint16_t text  = ThemeService::blend565(day.muted, night.muted, k);

    static constexpr int TIME_X = 42;
    static constexpr int TIME_Y = 4;
    static constexpr int TIME_W = 120;
    static constexpr int TIME_H = 8;

    _tft.fillRect(TIME_X, TIME_Y, TIME_W, TIME_H, bg);
    _tft.setTextColor(text, bg);
    _tft.setCursor(TIME_X, TIME_Y);
    _tft.print(buf);
}

// ============================================================================
// helpers
// ============================================================================

StatusBar::Status StatusBar::mapWifiStatus() const {
    if (!_wifi.isEnabled())
        return OFFLINE;

    switch (_wifi.state()) {
        case WifiService::State::CONNECTING: return CONNECTING;
        case WifiService::State::ONLINE:     return ONLINE;
        case WifiService::State::ERROR:      return ERROR;
        case WifiService::State::OFF:
        default:                             return OFFLINE;
    }
}

StatusBar::Status StatusBar::mapTimeStatus() const {
    return _time.isValid() ? ONLINE : OFFLINE;
}

// ---------------------------------------------------------------------------

uint16_t StatusBar::statusDotColor(Status s) const {

    const Theme& day   = THEME_DAY;
    const Theme& night = THEME_NIGHT;
    float k = _night.value();

    switch (s) {
        case ONLINE:
            return ThemeService::blend565(day.textSecondary, night.textSecondary, k);
        case CONNECTING:
            return ThemeService::blend565(day.accent, night.accent, k);
        case ERROR:
            return ThemeService::blend565(day.error, night.error, k);
        case OFFLINE:
        default:
            return ThemeService::blend565(day.muted, night.muted, k);
    }
}

// ---------------------------------------------------------------------------

void StatusBar::drawDot(int cx, int cy, uint16_t color) {
    _tft.fillCircle(cx, cy, 2, color);
}

// ---------------------------------------------------------------------------
// Weekday names
// ---------------------------------------------------------------------------

const char* StatusBar::weekdayEnFromTm(const tm& t) const {

    static const char* NAMES[] = {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
    };

    if (t.tm_wday < 0 || t.tm_wday > 6)
        return "------";

    return NAMES[t.tm_wday];
}