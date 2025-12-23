#include "screens/SettingsScreen.h"
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// PreferencesService — глобальный объект
// ============================================================================
extern PreferencesService prefs;

// ============================================================================
// helpers
// ============================================================================
static void formatOffsetHM(int32_t sec, char* out, size_t outSz) {
    int32_t s = sec;
    char sign = '+';
    if (s < 0) { sign = '-'; s = -s; }
    snprintf(out, outSz, "%c%02d:%02d", sign, s / 3600, (s % 3600) / 60);
}

// ============================================================================
// REDRAW ALL
// ============================================================================
void SettingsScreen::redrawAll() {
    const Theme& th = theme();

    int h = _layout.buttonBarY();
    if (_needFullClear || _lastDrawnLevel != _level) {
        _tft.fillRect(0, 0, _tft.width(), h, th.bg);
        _needFullClear  = false;
        _lastDrawnLevel = _level;
    }

    switch (_level) {
        case Level::ROOT:     drawRoot();     break;
        case Level::TIME:     drawTime();     break;
        case Level::NIGHT:    drawNight();    break;
        case Level::TIMEZONE: drawTimezone(); break;
    }

    drawButtonHints();
}

// ============================================================================
// ROOT MENU
// ============================================================================
void SettingsScreen::drawRoot() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(20, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("SETTINGS");

    _tft.setTextSize(1);

    int top = 36;
    int bottom = _layout.buttonBarY();
    int count = sizeof(MENU) / sizeof(MENU[0]);
    int rowH = (bottom - top) / count;

    for (int i = 0; i < count; i++) {
        int y = top + i * rowH;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        bool sel = (i == _selected);

        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print(MENU[i].label);
    }
}

// ============================================================================
// TIME
// ============================================================================
void SettingsScreen::drawTime() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(40, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Time");

    _tft.setTextSize(1);

    int y = 40;
    _tft.fillRect(0, y, _tft.width(), 18, th.bg);

    bool sel = (_subSelected == 0);

    _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
    _tft.setCursor(12, y + 4);
    _tft.print(sel ? "> " : "  ");
    _tft.print("Source: ");

    _tft.setTextColor(
        (sel && _mode == UiMode::EDIT) ? th.warn : th.textPrimary,
        th.bg
    );

    _tft.print(
        _tmpTimeMode == TimeService::AUTO      ? "AUTO"  :
        _tmpTimeMode == TimeService::RTC_ONLY ? "RTC"   :
        _tmpTimeMode == TimeService::NTP_ONLY ? "NTP"   :
                                                 "LOCAL"
    );
}

// ============================================================================
// NIGHT MODE
// ============================================================================
void SettingsScreen::drawNight() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(24, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Night mode");

    _tft.setTextSize(1);

    int top = 40;
    int rowH = 18;
    char buf[8];

    for (int i = 0; i < 3; i++) {
        int y = top + i * rowH;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        bool sel = (_subSelected == i);

        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");

        if (i == 0) {
            _tft.print("Mode: ");
            _tft.setTextColor(
                (sel && _mode == UiMode::EDIT) ? th.warn : th.textPrimary,
                th.bg
            );
            _tft.print(
                _tmpMode == NightService::Mode::AUTO ? "AUTO" :
                _tmpMode == NightService::Mode::ON   ? "ON"   : "OFF"
            );
        } else {
            int v = (i == 1) ? _tmpNightStart : _tmpNightEnd;
            snprintf(buf, sizeof(buf), "%02d:%02d", v / 60, v % 60);

            _tft.print(i == 1 ? "Start: " : "End:   ");
            _tft.setTextColor(
                (sel && _mode == UiMode::EDIT) ? th.warn : th.textPrimary,
                th.bg
            );
            _tft.print(buf);
        }
    }
}

// ============================================================================
// TIMEZONE
// ============================================================================
void SettingsScreen::drawTimezone() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(18, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Timezone");

    _tft.setTextSize(1);

    int top = 40;
    int rowH = 18;

    int32_t utc = (_mode == UiMode::EDIT) ? _tmpTzSec  : prefs.tzGmtOffset();
    int32_t dst = (_mode == UiMode::EDIT) ? _tmpDstSec : prefs.tzDstOffset();

    char ub[8], db[8];
    formatOffsetHM(utc, ub, sizeof(ub));
    formatOffsetHM(dst, db, sizeof(db));

    for (int i = 0; i < 2; i++) {
        int y = top + i * rowH;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        bool sel = (_subSelected == i);

        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print(i == 0 ? "UTC " : "DST ");

        _tft.setTextColor(
            (sel && _mode == UiMode::EDIT) ? th.warn : th.textPrimary,
            th.bg
        );
        _tft.print(i == 0 ? ub : db);
    }
}

// ============================================================================
// BUTTON HINTS
// ============================================================================
void SettingsScreen::drawButtonHints() {
    const Theme& th = theme();
    int y0 = _layout.buttonBarY();

    _tft.fillRect(0, y0, _tft.width(), _tft.height() - y0, th.bg);
    _tft.setTextSize(1);
    _tft.setCursor(4, y0 + 4);

    auto col = [&](HintBtn b) {
        return (_hintFlash > 0 && _pressedBtn == b)
            ? th.select
            : th.muted;
    };

    bool edit = (_mode == UiMode::EDIT);

    _tft.setTextColor(col(HintBtn::LEFT),  th.bg);  _tft.print("< ");
    _tft.setTextColor(col(HintBtn::RIGHT), th.bg);  _tft.print(">   ");
    _tft.setTextColor(col(HintBtn::OK),    th.bg);  _tft.print(edit ? "OK+  " : "OK   ");
    _tft.setTextColor(col(HintBtn::BACK),  th.bg);  _tft.print(edit ? "BACK+" : "BACK");
}