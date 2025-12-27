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
// Wi-Fi RSSI bars (UI responsibility)
// ============================================================================
static int rssiToBars(int rssi) {
    if (rssi >= -55) return 4;
    if (rssi >= -65) return 3;
    if (rssi >= -75) return 2;
    if (rssi >= -85) return 1;
    return 0;
}

static void drawRssiBars(
    Adafruit_ST7735& tft,
    const Theme& th,
    int x,
    int yMid,
    int rssi
) {
    const int bw   = 2;
    const int gap  = 1;
    const int bars = 4;

    int filled = rssiToBars(rssi);

    for (int i = 0; i < bars; i++) {
        int barH = 2 + i * 2;
        int bx   = x + i * (bw + gap);
        int by   = yMid - barH;

        uint16_t col = (i < filled) ? th.textPrimary : th.muted;
        tft.fillRect(bx, by, bw, barH, col);
    }
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
        bool sel = (i == _selected);

        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);
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
            _tft.print(_tmpWifiOn ? "ON" : "OFF");
        } else {
            _tft.print("Scan");
        }
    }
}

// ============================================================================
// WIFI LIST — ИСПРАВЛЕННЫЙ ПО КОНТРАКТУ
// ============================================================================
void SettingsScreen::drawWifiList() {

    const Theme& th = theme();

    int listTop = 36;
    int rowH    = 16;
    int listH   = _layout.buttonBarY() - listTop;

    _tft.fillRect(0, listTop, _tft.width(), listH, th.bg);

    _tft.setTextSize(2);
    _tft.setCursor(18, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Wi-Fi scan");

    _tft.setTextSize(1);

    if (_wifi.scanState() == WifiService::ScanState::SCANNING) {
        _tft.setCursor(20, 50);
        _tft.setTextColor(th.muted, th.bg);
        _tft.print("Scanning...");
        return;
    }

    constexpr int VISIBLE_ROWS = 4;
    constexpr int ICON_W = 12;

    int netCount = _wifi.networksCount();

    for (int i = 0; i < VISIBLE_ROWS; i++) {
        int idx = _wifiListTop + i;
        if (idx >= netCount) break;

        const WifiService::Network& net = _wifi.networkAt(idx);

        int rowY = listTop + i * rowH;
        bool sel = (idx == _wifiListSelected);

        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.setCursor(8, rowY + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print(net.ssid);

        int iconX = _tft.width() - ICON_W - 2;
        int yMid  = rowY + rowH / 2;
        drawRssiBars(_tft, th, iconX, yMid, net.rssi);

        if (net.connected) {
            _tft.setTextColor(th.textSecondary, th.bg);
            _tft.print(" [connected]");
        }
    }

    int rowY = listTop + VISIBLE_ROWS * rowH;
    bool sel = (_wifiListSelected == netCount);

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
    for (int i = 0; i < _wifiPassLen; i++)
        _tft.print('*');
}

// ============================================================================
// TIME / NIGHT / TIMEZONE
// ============================================================================
void SettingsScreen::drawTime() {
    const Theme& th = theme();
    _tft.setTextSize(2);
    _tft.setCursor(40, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Time");
}

void SettingsScreen::drawNight() {
    const Theme& th = theme();
    _tft.setTextSize(2);
    _tft.setCursor(24, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Night mode");
}

void SettingsScreen::drawTimezone() {
    const Theme& th = theme();
    _tft.setTextSize(2);
    _tft.setCursor(18, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Timezone");
}