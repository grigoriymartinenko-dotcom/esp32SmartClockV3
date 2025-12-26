#include "screens/SettingsScreen.h"
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

/*
 * SettingsDraw.cpp
 * ----------------
 * ВСЯ отрисовка экрана Settings.
 * НИКАКОЙ логики, ТОЛЬКО визуал.
 */

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
        case Level::ROOT:          drawRoot();         break;
        case Level::WIFI:          drawWifi();         break;
        case Level::WIFI_LIST:     drawWifiList();     break;
        case Level::WIFI_PASSWORD: drawWifiPassword(); break;
        case Level::TIME:          drawTime();         break;
        case Level::NIGHT:         drawNight();        break;
        case Level::TIMEZONE:      drawTimezone();     break;
    }

    }

// ============================================================================
// ROOT
// ============================================================================
void SettingsScreen::drawRoot() {

    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(20, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
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

        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print(MENU[i].label);
    }
}

// ============================================================================
// WIFI MENU
// ============================================================================
void SettingsScreen::drawWifi() {

    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(34, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Wi-Fi");

    _tft.setTextSize(1);

    int top  = 40;
    int rowH = 18;

    for (int i = 0; i < 2; i++) {
        int y = top + i * rowH;
        bool sel = (_subSelected == i);

        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");

        if (i == 0) {
            _tft.print("State: ");
            _tft.setTextColor(
                (sel && _mode == UiMode::EDIT) ? th.warn : th.textPrimary,
                th.bg
            );
            _tft.print(_tmpWifiOn ? "ON" : "OFF");
        } else {
            _tft.print("Scan");
        }
    }
}

// ============================================================================
// WIFI LIST
// ============================================================================
void SettingsScreen::drawWifiList() {

    const Theme& th = theme();

    int listTop = 36;
    int listH   = _layout.buttonBarY() - listTop;
    _tft.fillRect(0, listTop, _tft.width(), listH, th.bg);

    _tft.setTextSize(2);
    _tft.setCursor(18, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Wi-Fi scan");

    _tft.setTextSize(1);

    if (_wifi.isScanning()) {
        _tft.setCursor(20, 50);
        _tft.setTextColor(th.muted, th.bg);
        _tft.print("Scanning...");
        return;
    }

    int rowH = 16;
    int y    = listTop;

    const int netCount  = _wifi.networksCount();
    const int rescanIdx = netCount;

    if (_wifi.isScanFinished() && netCount == 0) {
        bool sel = (_wifiListSelected == 0);
        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.setCursor(8, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("|-----Rescan-----|");
        return;
    }

    constexpr int VISIBLE_ROWS = 4;

    for (int i = 0; i < VISIBLE_ROWS; i++) {
        int idx = _wifiListTop + i;
        if (idx >= netCount) break;

        int rowY = listTop + i * rowH;
        bool sel = (idx == _wifiListSelected);

        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.setCursor(8, rowY + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print(_wifi.ssidAt(idx));
    }

    int rowY = listTop + VISIBLE_ROWS * rowH;
    bool sel = (_wifiListSelected == rescanIdx);

    _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
    _tft.setCursor(8, rowY + 4);
    _tft.print(sel ? "> " : "  ");
    _tft.print("|-----Rescan-----|");
}

// ============================================================================
// WIFI PASSWORD
// ============================================================================
void SettingsScreen::drawWifiPassword() {

    const Theme& th = theme();
    int h = _layout.buttonBarY();

    _tft.fillRect(0, 0, _tft.width(), h, th.bg);

    _tft.setTextSize(2);
    _tft.setCursor(10, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Wi-Fi pass");

    _tft.setTextSize(1);

    _tft.setCursor(10, 40);
    _tft.print("Char:");
    _tft.setCursor(60, 40);
    _tft.setTextColor(th.select, th.bg);
    _tft.print(PASS_CHARS[_wifiCharIdx]);

    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(10, 60);
    _tft.print("Pass:");
    _tft.setCursor(60, 60);
    for (int i = 0; i < _wifiPassLen; i++)
        _tft.print('*');
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
}

// ============================================================================
// NIGHT
// ============================================================================
void SettingsScreen::drawNight() {

    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(24, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Night mode");
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
}

// ============================================================================
// BUTTON HINTS
// ============================================================================
