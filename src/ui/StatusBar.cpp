#include "ui/StatusBar.h"
#include <stdio.h>

/*
 * StatusBar.cpp
 * -------------
 * Двухстрочный статусбар:
 *  строка 1 — WiFi + дата
 *  строка 2 — NTP  + день недели
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

// --------------------------------------------------
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

// --------------------------------------------------
void StatusBar::draw() {

    const Theme& th = _theme.current();

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextWrap(false);

    _tft.fillRect(0, 0, _tft.width(), HEIGHT, th.bg);

    const int Y1 = 4;
    const int Y2 = 14;
    const int DOT_X = 4;

    // ===== LINE 1: WiFi =====
    drawDot(DOT_X, Y1 + 4, statusDotColor(_wifi, th));

    _tft.setTextColor(th.muted, th.bg);
    _tft.setCursor(10, Y1);
    _tft.print("WiFi");

    if (_time.isValid()) {
        tm t{};
        if (_time.getTm(t)) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%02d.%02d.%04d",
                t.tm_mday, t.tm_mon + 1, t.tm_year + 1900
            );
            int w = strlen(buf) * 6;
            _tft.setCursor(_tft.width() - w - 2, Y1);
            _tft.print(buf);
        }
    }

    // ===== LINE 2: NTP / RTC =====
    drawDot(DOT_X, Y2 + 4, statusDotColor(_ntp, th));

    _tft.setTextColor(th.muted, th.bg);
    _tft.setCursor(10, Y2);
    _tft.print(
        (_time.source() == TimeService::NTP) ? "NTP" :
        (_time.source() == TimeService::RTC) ? "RTC" : "---"
    );
}

// --------------------------------------------------
void StatusBar::drawDot(int cx, int cy, uint16_t color) {
    _tft.fillCircle(cx, cy, 2, color);
}

uint16_t StatusBar::statusDotColor(Status s, const Theme& th) const {
    switch (s) {
        case ONLINE:     return th.textSecondary;
        case CONNECTING: return th.accent;
        case ERROR:      return th.error;
        case OFFLINE:
        default:         return th.muted;
    }
}