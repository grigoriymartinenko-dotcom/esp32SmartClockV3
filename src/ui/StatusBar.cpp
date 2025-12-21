#include "ui/StatusBar.h"
#include <stdio.h>

/*
 * StatusBar.cpp
 * -------------
 * Полностью реактивная статусная панель.
 * Никаких таймеров, никаких задержек.
 */

StatusBar::StatusBar(
    Adafruit_ST7735& tft,
    ThemeService& theme,
    TimeService& time
)
: _tft(tft)
, _theme(theme)
, _time(time)
{}

// ------------------------------------------------------------
// public API
// ------------------------------------------------------------
void StatusBar::markDirty() {
    _dirty = true;
}

void StatusBar::setWiFiStatus(Status s) {
    if (_wifi != s) {
        _wifi = s;
        markDirty();
    }
}

void StatusBar::setNtpStatus(Status s) {
    if (_ntp != s) {
        _ntp = s;
        markDirty();
    }
}

void StatusBar::update() {
    if (!_dirty) return;
    _dirty = false;
    draw();
}

// ------------------------------------------------------------
// draw
// ------------------------------------------------------------
void StatusBar::draw() {

    const Theme& th = _theme.current();

    // --- reset GFX ---
    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextWrap(false);

    // --- background ---
    _tft.fillRect(0, 0, _tft.width(), HEIGHT, th.bg);

    // =========================================================
    // LEFT: WiFi
    // =========================================================
    _tft.setTextColor(statusColor(_wifi, th), th.bg);
    _tft.setCursor(2, 6);
    _tft.print("WiFi:");
    _tft.print(statusText(_wifi));

    // =========================================================
    // CENTER: DATE
    // =========================================================
    if (_time.isValid()) {
        char buf[12];
        snprintf(
            buf,
            sizeof(buf),
            "%02d.%02d.%04d",
            _time.day(),
            _time.month(),
            _time.year()
        );

        int16_t x1, y1;
        uint16_t w, h;
        _tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);

        _tft.setTextColor(th.textSecondary, th.bg);
        _tft.setCursor((_tft.width() - w) / 2, 6);
        _tft.print(buf);
    }

    // =========================================================
    // RIGHT: TIME SOURCE (RTC / NTP)
    // =========================================================
    TimeService::Source src = _time.source();

    uint16_t col =
        (src == TimeService::NTP) ? th.textPrimary :
        (src == TimeService::RTC) ? th.textSecondary :
                                    th.muted;

    _tft.setTextColor(col, th.bg);
    _tft.setCursor(_tft.width() - 28, 6);
    _tft.print(timeSourceText(src));
}

// ------------------------------------------------------------
// helpers
// ------------------------------------------------------------
const char* StatusBar::statusText(Status s) const {
    switch (s) {
        case ONLINE:     return "+";
        case CONNECTING: return "*";
        case ERROR:      return "!";
        case OFFLINE:
        default:         return "-";
    }
}

uint16_t StatusBar::statusColor(Status s, const Theme& th) const {
    switch (s) {
        case ERROR:      return ST7735_RED;
        case CONNECTING: return th.accent;
        case ONLINE:     return th.textPrimary;
        case OFFLINE:
        default:         return th.textSecondary;
    }
}

const char* StatusBar::timeSourceText(TimeService::Source s) const {
    switch (s) {
        case TimeService::RTC: return "RTC";
        case TimeService::NTP: return "NTP";
        case TimeService::NONE:
        default:               return "---";
    }
}