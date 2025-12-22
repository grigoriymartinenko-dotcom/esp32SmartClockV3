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
// small helpers (draw only)
// ============================================================================
static void formatOffsetHM(int32_t sec, char* out, size_t outSz) {
    int32_t s = sec;
    char sign = '+';
    if (s < 0) { sign = '-'; s = -s; }

    int hh = (int)(s / 3600);
    int mm = (int)((s % 3600) / 60);

    snprintf(out, outSz, "%c%02d:%02d", sign, hh, mm);
}

// ============================================================================
// DRAW
// ============================================================================
void SettingsScreen::redrawAll() {
    const Theme& th = theme();

    int yStart = 0;
    int h      = _layout.buttonBarY() - yStart;

    if (_needFullClear || _lastDrawnLevel != _level) {
        _tft.fillRect(0, yStart, _tft.width(), h, th.bg);
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
// ROOT
// ============================================================================
void SettingsScreen::drawRoot() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(20, 8);
    _tft.print("SETTINGS");

    _tft.setTextSize(1);

    int top    = 36;
    int bottom = _layout.buttonBarY();
    int count  = sizeof(MENU) / sizeof(MENU[0]);
    int rowH   = (bottom - top) / count;

    for (int i = 0; i < count; i++) {
        int y = top + i * rowH;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        bool sel = (i == _selected);

        _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
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
    _tft.setCursor(40, 8);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Time");

    int top = 40;
    int rowH = 18;

    _tft.fillRect(0, top, _tft.width(), rowH, th.bg);

    bool sel = (_subSelected == 0);
    _tft.setTextSize(1);
    _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
    _tft.setCursor(12, top + 4);
    _tft.print(sel ? "> " : "  ");

    _tft.print("Source: ");

    uint16_t valCol = (_mode == UiMode::EDIT && sel) ? th.error
                                                     : (sel ? ST77XX_GREEN : th.textPrimary);
    _tft.setTextColor(valCol, th.bg);

    const char* txt =
        (_tmpTimeMode == TimeService::AUTO)      ? "AUTO"  :
        (_tmpTimeMode == TimeService::RTC_ONLY) ? "RTC"   :
        (_tmpTimeMode == TimeService::NTP_ONLY) ? "NTP"   :
                                                   "LOCAL";

    _tft.print(txt);
}

// ============================================================================
// NIGHT (3 items: Mode / Start / End)
// ============================================================================
void SettingsScreen::drawNight() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(24, 8);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Night mode");

    int top = 40;
    int rowH = 18;

    char buf[8];

    // --- Row 0: Mode ---
    {
        int y = top;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);
        bool sel = (_subSelected == 0);

        _tft.setTextSize(1);
        _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("Mode: ");

        uint16_t col =
            (_mode == UiMode::EDIT && sel) ? th.error :
            (sel ? ST77XX_GREEN : th.textPrimary);

        _tft.setTextColor(col, th.bg);

        _tft.print(
            _tmpMode == NightService::Mode::AUTO ? "AUTO" :
            _tmpMode == NightService::Mode::ON   ? "ON"   : "OFF"
        );
    }

    // --- Row 1: Start ---
    {
        int y = top + rowH;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);
        bool sel = (_subSelected == 1);

        snprintf(buf, sizeof(buf), "%02d:%02d", _tmpNightStart / 60, _tmpNightStart % 60);

        _tft.setTextSize(1);
        _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("Start: ");

        uint16_t col = (_mode == UiMode::EDIT && sel) ? th.error : th.textPrimary;
        _tft.setTextColor(col, th.bg);
        _tft.print(buf);
    }

    // --- Row 2: End ---
    {
        int y = top + rowH * 2;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);
        bool sel = (_subSelected == 2);

        snprintf(buf, sizeof(buf), "%02d:%02d", _tmpNightEnd / 60, _tmpNightEnd % 60);

        _tft.setTextSize(1);
        _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("End:   ");

        uint16_t col = (_mode == UiMode::EDIT && sel) ? th.error : th.textPrimary;
        _tft.setTextColor(col, th.bg);
        _tft.print(buf);
    }
}

// ============================================================================
// TIMEZONE
// ============================================================================
void SettingsScreen::drawTimezone() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(18, 8);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Timezone");

    int top = 40;
    int rowH = 18;

    int32_t utcSec = (_mode == UiMode::EDIT) ? _tmpTzSec  : prefs.tzGmtOffset();
    int32_t dstSec = (_mode == UiMode::EDIT) ? _tmpDstSec : prefs.tzDstOffset();

    char utcBuf[8];
    char dstBuf[8];
    formatOffsetHM(utcSec, utcBuf, sizeof(utcBuf));
    formatOffsetHM(dstSec, dstBuf, sizeof(dstBuf));

    // --- Row 0: UTC ---
    {
        int y = top + 0 * rowH;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        bool sel = (_subSelected == 0);

        _tft.setTextSize(1);
        _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("UTC ");

        uint16_t valCol =
            (_mode == UiMode::EDIT && sel)
                ? th.error
                : (sel ? ST77XX_GREEN : th.textPrimary);

        _tft.setTextColor(valCol, th.bg);
        _tft.print(utcBuf);
    }

    // --- Row 1: DST ---
    {
        int y = top + 1 * rowH;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        bool sel = (_subSelected == 1);

        _tft.setTextSize(1);
        _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("DST ");

        uint16_t valCol =
            (_mode == UiMode::EDIT && sel)
                ? th.error
                : (sel ? ST77XX_GREEN : th.textPrimary);

        _tft.setTextColor(valCol, th.bg);
        _tft.print(dstBuf);
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
            ? ST77XX_GREEN
            : th.muted;
    };

    _tft.setTextColor(col(HintBtn::LEFT),  th.bg); _tft.print("< ");
    _tft.setTextColor(col(HintBtn::RIGHT), th.bg); _tft.print(">   ");
    _tft.setTextColor(col(HintBtn::OK),    th.bg); _tft.print("OK   ");
    _tft.setTextColor(col(HintBtn::BACK),  th.bg); _tft.print("BACK");
}