#include "screens/SettingsScreen.h"
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <cstring>   // 🔥 strlen / memset

/*
 * SettingsDraw.cpp
 * ----------------
 * ВСЯ отрисовка экрана Settings.
 *
 * ПРАВИЛА:
 *  - НИКАКОЙ логики изменения состояния
 *  - НИКАКОГО чтения кнопок
 *  - ТОЛЬКО визуализация текущего состояния UI
 *
 * ВАЖНО:
 *  Если пункт существует в логике (SettingsScreen.cpp),
 *  он ОБЯЗАН быть отрисован здесь.
 *  Иначе пользователь попадает на "невидимое меню".
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

    // Полная очистка экрана ТОЛЬКО при смене уровня меню или темы
    if (_needFullClear || _lastDrawnLevel != _level) {
        _tft.fillRect(0, 0, _tft.width(), h, th.bg);
        _needFullClear  = false;
        _lastDrawnLevel = _level;
    }

    switch (_level) {
        case Level::ROOT:      drawRoot();         break;
        case Level::WIFI:      drawWifi();         break;
        case Level::WIFI_LIST: drawWifiList();     break;
        case Level::WIFI_PASSWORD: drawWifiPassword(); break; // ✅ FIX: парольный экран
        case Level::TIME:      drawTime();         break;
        case Level::NIGHT:     drawNight();        break;
        case Level::TIMEZONE:  drawTimezone();     break;
    }

    drawButtonHints();
}

// ============================================================================
// WIFI MENU (State + Scan)
// ============================================================================
void SettingsScreen::drawWifi() {

    const Theme& th = theme();

    // ----- Title -----
    _tft.setTextSize(2);
    _tft.setCursor(34, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Wi-Fi");

    _tft.setTextSize(1);

    int top  = 40;
    int rowH = 18;

    // ------------------------------------------------------------------------
    // ROW 0 — State: ON / OFF
    // ------------------------------------------------------------------------
    {
        int y = top + 0 * rowH;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        bool sel = (_subSelected == 0);

        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("State: ");

        _tft.setTextColor(
            (sel && _mode == UiMode::EDIT) ? th.warn : th.textPrimary,
            th.bg
        );
        _tft.print(_tmpWifiOn ? "ON" : "OFF");
    }

    // ------------------------------------------------------------------------
    // ROW 1 — Scan
    // ------------------------------------------------------------------------
    {
        int y = top + 1 * rowH;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        bool sel = (_subSelected == 1);

        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("Scan");
    }
}

// ============================================================================
// WIFI LIST (SSID + RESCAN)
// ============================================================================
void SettingsScreen::drawWifiList() {

    const Theme& th = theme();

    // ------------------------------------------------------------------------
    // Очищаем ВСЮ рабочую область списка
    // (обязательно для async-экранов)
    // ------------------------------------------------------------------------
    int listTop = 36;
    int listH   = _layout.buttonBarY() - listTop;
    _tft.fillRect(0, listTop, _tft.width(), listH, th.bg);

    // ----- Title -----
    _tft.setTextSize(2);
    _tft.setCursor(18, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Wi-Fi scan");

    _tft.setTextSize(1);

    // ------------------------------------------------------------------------
    // 1) SCANNING — высший приоритет
    // ------------------------------------------------------------------------
    if (_wifi.isScanning()) {
        _tft.setCursor(20, 50);
        _tft.setTextColor(th.muted, th.bg);
        _tft.print("Scanning...");
        return;
    }

    int rowH = 16;
    int y    = listTop;
    int idx  = 0;

    // ------------------------------------------------------------------------
    // 2) SCAN FINISHED, NO NETWORKS → показываем ТОЛЬКО Rescan
    // ------------------------------------------------------------------------
    if (_wifi.isScanFinished() && _wifi.networksCount() == 0) {

        bool sel = (_wifiListSelected == 0);

        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.setCursor(8, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("|-----Rescan-----|");

        return;
    }

    // ------------------------------------------------------------------------
    // 3) NETWORK LIST
    // ------------------------------------------------------------------------
    constexpr int VISIBLE_ROWS = 4;

    for (int i = 0; i < VISIBLE_ROWS; i++) {

        int idx = _wifiListTop + i;

        if (idx >= _wifi.networksCount())
            break;

        int y = listTop + i * rowH;

        bool sel = (idx == _wifiListSelected);

        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.setCursor(8, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print(_wifi.ssidAt(idx));
    }

    // ------------------------------------------------------------------------
    // 4) RESCAN (всегда последний пункт)
    // ------------------------------------------------------------------------
    {
        bool sel = (_wifiListSelected == idx);

        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.setCursor(8, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("|-----Rescan-----|");
    }
    // ------------------------------------------------------------------------
    // CONNECTION STATUS (CONNECTING / ONLINE / ERROR)
    // ------------------------------------------------------------------------
    WifiService::State st = _wifi.state();

    int yStatus = _layout.buttonBarY() - 32;

    _tft.fillRect(0, yStatus, _tft.width(), 16, th.bg);
    _tft.setCursor(10, yStatus + 4+18);

    switch (st) {
        case WifiService::State::CONNECTING:
            _tft.setTextColor(th.warn, th.bg);
            _tft.print("Connecting...");
            break;

        case WifiService::State::ONLINE:
            _tft.setTextColor(th.textPrimary, th.bg);
            _tft.print("Connected");
            break;

        case WifiService::State::ERROR:
            _tft.setTextColor(th.error, th.bg);
            _tft.print("Connection error");
            break;

        default:
            break;
    }
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

    int top  = 40;
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
                _tmpMode == NightService::Mode::ON   ? "ON"   :
                                                       "OFF"
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

    int top  = 40;
    int rowH = 18;

    char ub[8], db[8];
    formatOffsetHM(_tmpTzSec,  ub, sizeof(ub));
    formatOffsetHM(_tmpDstSec, db, sizeof(db));

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

    _tft.setTextColor(col(HintBtn::LEFT),  th.bg); _tft.print("< ");
    _tft.setTextColor(col(HintBtn::RIGHT), th.bg); _tft.print(">   ");
    _tft.setTextColor(col(HintBtn::OK),    th.bg); _tft.print(edit ? "OK+  " : "OK   ");
    _tft.setTextColor(col(HintBtn::BACK),  th.bg); _tft.print(edit ? "BACK+" : "BACK");
}

void SettingsScreen::drawWifiPassword() {

    const Theme& th = theme();

    // ✅ очищаем рабочую область (всё кроме нижнего бара)
    int top = 0;
    int h   = _layout.buttonBarY();
    _tft.fillRect(0, top, _tft.width(), h, th.bg);

    _tft.setTextSize(2);
    _tft.setCursor(10, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Wi-Fi pass");

    _tft.setTextSize(1);

    _tft.setCursor(10, 40);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Char:");
    _tft.setCursor(60, 40);
    _tft.setTextColor(th.select, th.bg);
    _tft.print(PASS_CHARS[_wifiCharIdx]);

    _tft.setCursor(10, 60);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Pass:");

    _tft.setCursor(60, 60);
    _tft.setTextColor(th.textPrimary, th.bg);
    for (int i = 0; i < _wifiPassLen; i++)
        _tft.print('*');
}