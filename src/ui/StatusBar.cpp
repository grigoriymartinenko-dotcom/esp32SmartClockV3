#include "ui/StatusBar.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
{
}

// ============================================================================
// public
// ============================================================================

void StatusBar::markDirty() {
    _dirty = true;
}

void StatusBar::update() {

    Status newWifi = mapWifiStatus();
    Status newTime = mapTimeStatus();

    if (newWifi != _wifiSt || newTime != _timeSt) {
        _wifiSt = newWifi;
        _timeSt = newTime;
        _dirty  = true;
    }

    if (_dirty) {
        _dirty = false;

        // ❗ ТОЛЬКО стабильный ThemeBlend
        const ThemeBlend& th = _theme.blend();
        drawStatic(th);
    }
}

// ============================================================================
// drawing
// ============================================================================

void StatusBar::drawStatic(const ThemeBlend& th) {

    // ------------------------------------------------------------------------
    // BACKGROUND
    // ------------------------------------------------------------------------
    _tft.fillRect(0, 0, _tft.width(), HEIGHT, th.bg);

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextWrap(false);

    // ------------------------------------------------------------------------
    // STATUS DOTS
    // ------------------------------------------------------------------------
    drawDot(4, 8,  statusDotColor(_wifiSt, th));
    drawDot(4, 18, statusDotColor(_timeSt, th));

    // ------------------------------------------------------------------------
    // WIFI LABEL
    // ------------------------------------------------------------------------
    _tft.setTextColor(th.muted, th.bg);
    _tft.setCursor(10, 4);
    _tft.print("WiFi");

    // ------------------------------------------------------------------------
    // TIME SOURCE (RTC / NTP / NTP… / ERR)
    // ------------------------------------------------------------------------
    char src[6] = {0};

    if (_time.syncState() == TimeService::ERROR) {
        strcpy(src, "ERR");
    }
    else if (_time.syncState() == TimeService::SYNCING) {
        snprintf(src, sizeof(src), "%s…", _time.sourceLabel());
    }
    else {
        snprintf(src, sizeof(src), "%s", _time.sourceLabel());
    }

    uint16_t srcColor = th.muted;
    if (_time.syncState() == TimeService::SYNCING) srcColor = th.accent;
    else if (_time.syncState() == TimeService::ERROR) srcColor = th.warn;
    else if (_time.source() == TimeService::NTP) srcColor = th.success;

    _tft.setTextColor(srcColor, th.bg);
    _tft.setCursor(10, 14);
    _tft.print(src);

    // ------------------------------------------------------------------------
    // DATE / WEEKDAY (РИСУЕТСЯ ТОЛЬКО ТУТ)
    // ------------------------------------------------------------------------
    if (_time.isValid()) {

        tm t{};
        if (_time.getTm(t)) {

            mktime(&t);

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

            _tft.setTextColor(th.muted, th.bg);
            _tft.setCursor(42, 4);
            _tft.print(buf);
        }
    }
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
        default:                             return OFFLINE;
    }
}

StatusBar::Status StatusBar::mapTimeStatus() const {

    switch (_time.syncState()) {
        case TimeService::SYNCING: return CONNECTING;
        case TimeService::ERROR:   return ERROR;
        default:                   break;
    }

    return _time.isValid() ? ONLINE : OFFLINE;
}

uint16_t StatusBar::statusDotColor(Status s, const ThemeBlend& th) const {
    switch (s) {
        case ONLINE:     return th.success;
        case CONNECTING: return th.accent;
        case ERROR:      return th.warn;
        case OFFLINE:
        default:         return th.muted;
    }
}

void StatusBar::drawDot(int cx, int cy, uint16_t color) {
    _tft.fillCircle(cx, cy, 2, color);
}

const char* StatusBar::weekdayEnFromTm(const tm& t) const {
    static const char* NAMES[] = {
        "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"
    };
    return (t.tm_wday >= 0 && t.tm_wday <= 6) ? NAMES[t.tm_wday] : "";
}