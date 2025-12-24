#include "screens/SettingsScreen.h"
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

/*
 * SettingsDraw.cpp
 * ----------------
 * ВСЯ отрисовка экрана Settings.
 *
 * ПРАВИЛА:
 *  - НИКАКОЙ логики изменения значений
 *  - НИКАКОГО чтения кнопок
 *  - ТОЛЬКО визуализация текущего UI-состояния
 *
 * Источники данных:
 *  - _level        — какой экран сейчас
 *  - _mode         — NAV / EDIT
 *  - _selected     — пункт в ROOT
 *  - _subSelected  — пункт в submenu
 *  - _tmp*         — временные значения (EDIT)
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

    // Высота области без BottomBar
    int h = _layout.buttonBarY();

    // Полная очистка нужна:
    //  - при первом входе
    //  - при смене уровня (ROOT → WIFI и т.п.)
    if (_needFullClear || _lastDrawnLevel != _level) {
        _tft.fillRect(0, 0, _tft.width(), h, th.bg);
        _needFullClear  = false;
        _lastDrawnLevel = _level;
    }

    // === Выбор отрисовки по текущему уровню ===
    switch (_level) {
        case Level::ROOT:     drawRoot();     break;
        case Level::WIFI:     drawWifi();     break;
        case Level::TIME:     drawTime();     break;
        case Level::NIGHT:    drawNight();    break;
        case Level::TIMEZONE: drawTimezone(); break;
    }

    // Подсказки кнопок рисуются ВСЕГДА
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
// WIFI
// ============================================================================
void SettingsScreen::drawWifi() {
    const Theme& th = theme();

    // ----- Заголовок -----
    _tft.setTextSize(2);
    _tft.setCursor(34, 10);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Wi-Fi");

    _tft.setTextSize(1);

    // ----- Единственный пункт меню -----
    int y = 40;
    _tft.fillRect(0, y, _tft.width(), 18, th.bg);

    bool sel = (_subSelected == 0);

    // Курсор
    _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
    _tft.setCursor(12, y + 4);
    _tft.print(sel ? "> " : "  ");
    _tft.print("State: ");

    /*
     * Цвет значения:
     *  - NAV  → обычный текст
     *  - EDIT → warn (чётко видно, что редактируем)
     */
    _tft.setTextColor(
        (sel && _mode == UiMode::EDIT) ? th.warn : th.textPrimary,
        th.bg
    );

    _tft.print(_tmpWifiOn ? "ON" : "OFF");

    // ----- Текущий РЕАЛЬНЫЙ статус Wi-Fi -----
    // (не редактируемый, только для информации)
    int y2 = y + 22;
    _tft.fillRect(0, y2, _tft.width(), 14, th.bg);

    _tft.setCursor(12, y2 + 2);
    _tft.setTextColor(th.muted, th.bg);
    _tft.print("Status: ");

    switch (_wifi.state()) {
        case WifiService::State::ONLINE:
            _tft.setTextColor(th.textSecondary, th.bg);
            _tft.print("ONLINE");
            break;

        case WifiService::State::CONNECTING:
            _tft.setTextColor(th.accent, th.bg);
            _tft.print("CONNECTING");
            break;

        case WifiService::State::ERROR:
            _tft.setTextColor(th.error, th.bg);
            _tft.print("ERROR");
            break;

        case WifiService::State::OFF:
        default:
            _tft.setTextColor(th.muted, th.bg);
            _tft.print("OFF");
            break;
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