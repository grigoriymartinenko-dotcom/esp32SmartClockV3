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
    ColorTemperatureService& colorTemp,
    TimeService& time,
    WifiService& wifi
)
: _tft(tft)
, _theme(theme)
, _night(nightTransition)
, _temp(colorTemp)
, _time(time)
, _wifi(wifi)
{}

// ============================================================================

void StatusBar::markDirty() {
    _dirty = true;
}

// ============================================================================

void StatusBar::update() {

    Status newWifi = mapWifiStatus();
    Status newTime = mapTimeStatus();

    if (newWifi != _wifiSt || newTime != _timeSt) {
        _wifiSt = newWifi;
        _timeSt = newTime;
        _dirty = true;
    }

    if (!_dirty) return;
    _dirty = false;

    ThemeBlend th = _theme.interpolate(_night.value());
    th = _temp.apply(th);

    drawStatic(th);
}

// ============================================================================

void StatusBar::drawStatic(const ThemeBlend& th) {

    _tft.fillRect(0, 0, _tft.width(), HEIGHT, th.bg);

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextWrap(false);

    drawDot(4, 8,  statusDotColor(_wifiSt, th));
    drawDot(4, 18, statusDotColor(_timeSt, th));

    _tft.setTextColor(th.muted, th.bg);
    _tft.setCursor(10, 4);
    _tft.print("WiFi");

    _tft.setCursor(10, 14);
    _tft.print(
        (_time.source() == TimeService::NTP) ? "NTP" :
        (_time.source() == TimeService::RTC) ? "RTC" : "---"
    );

    _lastTimeStr[0] = '\0';
}

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

    ThemeBlend th = _theme.interpolate(_night.value());
    th = _temp.apply(th);

    _tft.fillRect(42, 4, 120, 8, th.bg);
    _tft.setTextColor(th.muted, th.bg);
    _tft.setCursor(42, 4);
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
        default:                             return OFFLINE;
    }
}

StatusBar::Status StatusBar::mapTimeStatus() const {
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

// ---------------------------------------------------------------------------

const char* StatusBar::weekdayEnFromTm(const tm& t) const {
    static const char* NAMES[] = {
        "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"
    };
    return (t.tm_wday >= 0 && t.tm_wday <= 6) ? NAMES[t.tm_wday] : "------";
}