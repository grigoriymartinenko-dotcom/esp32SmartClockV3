#include "ui/StatusBar.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// ctor
// ============================================================================
StatusBar::StatusBar(
    Adafruit_ST7735& tft,
    ThemeService& theme,
    TimeService& time,
    WifiService& wifi
)
: _tft(tft)
, _theme(theme)
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

    // Перерисовываем СТАТИЧЕСКИЙ слой
    // ТОЛЬКО если реально что-то изменилось
    if (newWifi != _wifiSt ||
        newTime != _timeSt ||
        th.bg != _lastBg)
    {
        _wifiSt = newWifi;
        _timeSt = newTime;
        _lastBg = th.bg;
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

    const Theme& th = _theme.current();

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextWrap(false);

    // Фон статусбара
    _tft.fillRect(0, 0, _tft.width(), HEIGHT, th.bg);

    const int Y1 = 4;
    const int Y2 = 14;
    const int DOT_X = 4;

    // --- точки статуса ---
    drawDot(DOT_X, Y1 + 4, statusDotColor(_wifiSt, th));
    drawDot(DOT_X, Y2 + 4, statusDotColor(_timeSt, th));

    // --- подписи ---
    _tft.setTextColor(th.muted, th.bg);

    _tft.setCursor(10, Y1);
    _tft.print("WiFi");

    _tft.setCursor(10, Y2);
    _tft.print(
        (_time.source() == TimeService::NTP) ? "NTP" :
        (_time.source() == TimeService::RTC) ? "RTC" : "---"
    );

    // При полной перерисовке
    // принудительно сбрасываем кэш даты
    _lastTimeStr[0] = '\0';
}

// ============================================================================
// dynamic layer (TIME only)
// ============================================================================
void StatusBar::drawTimeOnly() {

    if (!_time.isValid()) return;

    tm t{};
    if (!_time.getTm(t)) return;

    // Формируем СТРОКУ ЦЕЛИКОМ
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

    // 🔑 КЛЮЧЕВОЙ МОМЕНТ:
    // если строка НЕ изменилась — НИЧЕГО НЕ РИСУЕМ
    if (strcmp(buf, _lastTimeStr) == 0)
        return;

    strcpy(_lastTimeStr, buf);

    const Theme& th = _theme.current();

    // ------------------------------
    // ФИКСИРОВАННАЯ ОБЛАСТЬ
    // ------------------------------
    static constexpr int TIME_X = 42;   // 60 - (3 * 6) = 42
    static constexpr int TIME_Y = 4;
    static constexpr int TIME_W = 120;
    static constexpr int TIME_H = 8;

    _tft.fillRect(TIME_X, TIME_Y, TIME_W, TIME_H, th.bg);
    _tft.setTextColor(th.muted, th.bg);
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
uint16_t StatusBar::statusDotColor(Status s, const Theme& th) const {
    switch (s) {
        case ONLINE:     return th.textSecondary;
        case CONNECTING: return th.accent;
        case ERROR:      return th.error;
        case OFFLINE:
        default:         return th.muted;
    }
}

// ---------------------------------------------------------------------------
void StatusBar::drawDot(int cx, int cy, uint16_t color) {
    _tft.fillCircle(cx, cy, 2, color);
}

// ---------------------------------------------------------------------------
// Weekday names (EN, fixed width, no allocations)
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