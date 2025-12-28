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
 *
 * КЛЮЧЕВОЙ ФИКС:
 * --------------
 * Adafruit_GFX хранит состояние текста ГЛОБАЛЬНО:
 *   - font
 *   - textSize
 *   - textWrap
 *
 * Если не сбрасывать это состояние ПЕРЕД КАЖДЫМ логическим блоком,
 * возникают:
 *   - налезание строк
 *   - переносы
 *   - "прыгающая" геометрия
 *
 * Поэтому здесь:
 *   - ЖЁСТКИЙ reset GFX состояния:
 *       setFont(nullptr)
 *       setTextWrap(false)
 *       setTextSize(1)
 *   - reset делается:
 *       * в redrawAll()
 *       * в начале drawWifiList()
 *       * внутри drawRow() ПЕРЕД ЛЮБЫМ print()
 */

// ============================================================================
// layout constants (НЕ тянем StatusBar.h)
// ============================================================================
static constexpr int STATUSBAR_H = 24;

// ============================================================================
// helpers
// ============================================================================
static void formatOffsetHM(int32_t sec, char* out, size_t outSz) {
    int32_t s = sec;
    char sign = '+';
    if (s < 0) { sign = '-'; s = -s; }
    snprintf(out, outSz, "%c%02d:%02d", sign, s / 3600, (s % 3600) / 60);
}

// формат HH:MM из минут (0..1439)
static void formatHMFromMin(int minutes, char* out, size_t outSz) {
    if (minutes < 0) minutes = 0;
    if (minutes > 1439) minutes = 1439;
    int hh = minutes / 60;
    int mm = minutes % 60;
    snprintf(out, outSz, "%02d:%02d", hh, mm);
}

// ============================================================================
// Wi-Fi RSSI bars (UI responsibility)
// ============================================================================
static uint8_t rssiToBars(int16_t rssi) {
    if (rssi == WifiService::RSSI_UNKNOWN) return 0;
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
    int16_t rssi
) {
    const int bw   = 2;
    const int gap  = 1;
    const int bars = 4;

    uint8_t filled = rssiToBars(rssi);

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
    const int y0 = STATUSBAR_H;

    // ------------------------------------------------------------------------
    // 🔒 ГЛОБАЛЬНЫЙ RESET GFX СОСТОЯНИЯ (обязателен)
    // ------------------------------------------------------------------------
    _tft.setFont(nullptr);
    _tft.setTextWrap(false);
    _tft.setTextSize(1);

    // ------------------------------------------------------------------------
    // Полный клир рабочей области
    // ------------------------------------------------------------------------
    if (_needFullClear || _lastDrawnLevel != _level) {
        _tft.fillRect(0, y0, _tft.width(), _tft.height() - y0, th.bg);
        _needFullClear  = false;
        _lastDrawnLevel = _level;

        // сброс кешей Wi-Fi списка
        _lastWifiListTop      = -1;
        _lastWifiListSelected = -1;
        _lastWifiNetCount     = -1;
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
    const int y0 = STATUSBAR_H;

    // --- TITLE ---
    _tft.setFont(nullptr);
    _tft.setTextWrap(false);
    _tft.setTextSize(2);
    _tft.setCursor(20, y0 + 6);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("SETTINGS");

    // --- LIST ---
    _tft.setTextSize(1);

    int top    = y0 + 28;
    int count  = sizeof(MENU) / sizeof(MENU[0]);
    constexpr int rowH = 12;

    for (int i = 0; i < count; i++) {
        int y = top + i * rowH;
        bool sel = (i == _selected);

        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);
        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        const int textY = y + (rowH - 8) / 2;
        _tft.setCursor(12, textY);
        _tft.print(sel ? "> " : "  ");
        _tft.print(MENU[i].label);
    }
}

// ============================================================================
// WIFI MENU
// ============================================================================
void SettingsScreen::drawWifi() {
    const Theme& th = theme();
    const int y0 = STATUSBAR_H;

    // --- TITLE ---
    _tft.setFont(nullptr);
    _tft.setTextWrap(false);
    _tft.setTextSize(2);
    _tft.setCursor(34, y0 + 6);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Wi-Fi");

    // --- LIST ---
    _tft.setTextSize(1);

    int top  = y0 + 28;
    int rowH = 18;

    for (int i = 0; i < 2; i++) {
        int y = top + i * rowH;
        bool sel = (_subSelected == i);

        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);
        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        const int textY = y + (rowH - 8) / 2;
        _tft.setCursor(12, textY);
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
// WIFI LIST — PARTIAL REDRAW (ANTI-FLICKER)
// ============================================================================
void SettingsScreen::drawWifiList() {

    const Theme& th = theme();

    constexpr int ROW_H = 12;
    constexpr int VISIBLE_ROWS = 4;
    constexpr int ICON_W = 12;

    const int y0 = STATUSBAR_H;
    const int TITLE_Y  = y0 + 6;
    const int LIST_TOP = y0 + 28;

    // ------------------------------------------------------------------------
    // 🔒 RESET ПЕРЕД СПИСКОМ (обязательно)
    // ------------------------------------------------------------------------
    _tft.setFont(nullptr);
    _tft.setTextWrap(false);
    _tft.setTextSize(1);

    int netCount = _wifi.networksCount();

    bool full =
        _lastWifiListTop  != _wifiListTop ||
        _lastWifiNetCount != netCount ||
        _lastWifiListTop  < 0;

    // --- HEADER ---
    _tft.fillRect(0, y0, _tft.width(), LIST_TOP - y0, th.bg);
    _tft.setTextSize(2);
    _tft.setCursor(18, TITLE_Y);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Wi-Fi scan");

    // ⚠️ ОБЯЗАТЕЛЬНЫЙ возврат!
    _tft.setTextSize(1);

    if (full) {
        int listH = _layout.buttonBarY() - LIST_TOP;
        _tft.fillRect(0, LIST_TOP, _tft.width(), listH, th.bg);
    }

    if (_wifi.scanState() == WifiService::ScanState::SCANNING) {
        if (full) {
            _tft.setCursor(20, LIST_TOP + 14);
            _tft.setTextColor(th.muted, th.bg);
            _tft.print("Scanning...");
        }
        return;
    }

    // --- ROW DRAW ---
    auto drawRow = [&](int idx) {

        // 🔒 САМЫЙ ВАЖНЫЙ FIX — reset ПЕРЕД КАЖДОЙ строкой
        _tft.setFont(nullptr);
        _tft.setTextWrap(false);
        _tft.setTextSize(1);

        if (idx < _wifiListTop || idx >= _wifiListTop + VISIBLE_ROWS) return;
        if (idx >= netCount) return;

        int i = idx - _wifiListTop;
        int rowY = LIST_TOP + i * ROW_H;

        _tft.fillRect(0, rowY, _tft.width(), ROW_H, th.bg);

        const WifiService::Network& net = _wifi.networkAt(idx);
        bool sel = (idx == _wifiListSelected);

        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        const int textY = rowY + (ROW_H - 8) / 2;
        _tft.setCursor(8, textY);
        _tft.print(sel ? "> " : "  ");
        _tft.print(net.ssid);

        int iconX = _tft.width() - ICON_W - 2;
        int yMid  = rowY + ROW_H / 2;
        drawRssiBars(_tft, th, iconX, yMid, net.rssi);

        if (net.connected) {
            _tft.setTextColor(th.textSecondary, th.bg);
            _tft.print(" [connected]");
        }
    };

    if (full) {
        for (int i = 0; i < VISIBLE_ROWS; i++)
            drawRow(_wifiListTop + i);
    } else if (_lastWifiListSelected != _wifiListSelected) {
        drawRow(_lastWifiListSelected);
        drawRow(_wifiListSelected);
    }

    _lastWifiListTop      = _wifiListTop;
    _lastWifiListSelected = _wifiListSelected;
    _lastWifiNetCount     = netCount;
}

// ============================================================================
// WIFI PASSWORD
// ============================================================================
void SettingsScreen::drawWifiPassword() {
    const Theme& th = theme();
    const int y0 = STATUSBAR_H;
    const int h  = _layout.buttonBarY();

    _tft.fillRect(0, y0, _tft.width(), h - y0, th.bg);

    _tft.setFont(nullptr);
    _tft.setTextWrap(false);

    _tft.setTextSize(2);
    _tft.setCursor(10, y0 + 6);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Wi-Fi pass");

    _tft.setTextSize(1);

    _tft.setCursor(10, y0 + 36);
    _tft.print("Char:");
    _tft.setCursor(60, y0 + 36);
    _tft.setTextColor(th.select, th.bg);
    _tft.print(PASS_CHARS[_wifiCharIdx]);

    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(10, y0 + 56);
    for (int i = 0; i < _wifiPassLen; i++)
        _tft.print('*');
}

// ============================================================================
// TIME / NIGHT / TIMEZONE
// ============================================================================
void SettingsScreen::drawTime() {
    const Theme& th = theme();
    const int y0 = STATUSBAR_H;

    _tft.setFont(nullptr);
    _tft.setTextWrap(false);
    _tft.setTextSize(2);
    _tft.setCursor(40, y0 + 6);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Time");

    // (здесь позже сделаем как Night: поля + highlight)
}

// ============================================================================
// NIGHT MODE — FULL UI + EDIT HIGHLIGHT
// ============================================================================
// ============================================================================
// NIGHT MODE
// ============================================================================
void SettingsScreen::drawNight() {

    const Theme& th = theme();
    const int y0 = STATUSBAR_H;

    // ------------------------------------------------------------------------
    // TITLE
    // ------------------------------------------------------------------------
    _tft.setFont(nullptr);
    _tft.setTextWrap(false);
    _tft.setTextSize(2);
    _tft.setCursor(20, y0 + 6);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Night mode");

    // ------------------------------------------------------------------------
    // LIST
    // ------------------------------------------------------------------------
    _tft.setTextSize(1);

    constexpr int ROW_H = 14;
    int top = y0 + 30;

    // ---------- Row 0: Mode ----------
    {
        bool selected = (_subSelected == 0);
        bool editing  = selected && (_mode == UiMode::EDIT);

        uint16_t color = editing
            ? th.warn        // 🔴 EDIT = RED
            : (selected ? th.select : th.textPrimary);

        _tft.fillRect(0, top, _tft.width(), ROW_H, th.bg);
        _tft.setTextColor(color, th.bg);
        _tft.setCursor(10, top + 3);
        _tft.print("> Mode: ");

        switch (_tmpMode) {
            case NightService::Mode::AUTO: _tft.print("AUTO"); break;
            case NightService::Mode::ON:   _tft.print("ON");   break;
            case NightService::Mode::OFF:  _tft.print("OFF");  break;
        }
    }

    // ---------- Row 1: Start ----------
    {
        int y = top + ROW_H;
        bool enabled  = (_tmpMode == NightService::Mode::AUTO);
        bool selected = (_subSelected == 1);
        bool editing  = selected && (_mode == UiMode::EDIT);

        uint16_t color =
            !enabled ? th.muted :
            editing  ? th.warn :
            selected ? th.select :
                       th.textPrimary;

        _tft.fillRect(0, y, _tft.width(), ROW_H, th.bg);
        _tft.setTextColor(color, th.bg);
        _tft.setCursor(10, y + 3);
        _tft.print("  Start: ");

        char buf[6];
        snprintf(buf, sizeof(buf), "%02d:%02d",
                 _tmpNightStart / 60,
                 _tmpNightStart % 60);
        _tft.print(buf);
    }

    // ---------- Row 2: End ----------
    {
        int y = top + ROW_H * 2;
        bool enabled  = (_tmpMode == NightService::Mode::AUTO);
        bool selected = (_subSelected == 2);
        bool editing  = selected && (_mode == UiMode::EDIT);

        uint16_t color =
            !enabled ? th.muted :
            editing  ? th.warn :
            selected ? th.select :
                       th.textPrimary;

        _tft.fillRect(0, y, _tft.width(), ROW_H, th.bg);
        _tft.setTextColor(color, th.bg);
        _tft.setCursor(10, y + 3);
        _tft.print("  End:   ");

        char buf[6];
        snprintf(buf, sizeof(buf), "%02d:%02d",
                 _tmpNightEnd / 60,
                 _tmpNightEnd % 60);
        _tft.print(buf);
    }

    // ------------------------------------------------------------------------
    // ❌ НИКАКИХ ПОДПИСЕЙ КНОПОК ТУТ БОЛЬШЕ НЕТ
    // ButtonBar — единственный источник кнопок
    // ------------------------------------------------------------------------
}

// ============================================================================
void SettingsScreen::drawTimezone() {
    const Theme& th = theme();
    const int y0 = STATUSBAR_H;

    _tft.setFont(nullptr);
    _tft.setTextWrap(false);
    _tft.setTextSize(2);
    _tft.setCursor(18, y0 + 6);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Timezone");

    // (позже сделаем как Night: поля + highlight)
}