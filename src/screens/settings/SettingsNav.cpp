#include "screens/SettingsScreen.h"

/*
 * SettingsNav.cpp
 * ----------------
 * ВСЯ логика навигации (LEFT / RIGHT).
 *
 * ПРАВИЛА:
 *  - НИКАКОЙ отрисовки
 *  - НИКАКОЙ работы с WiFi / Preferences
 *  - ТОЛЬКО перемещение курсоров
 *
 * ВАЖНО:
 *  navLeft / navRight существуют ТОЛЬКО ЗДЕСЬ.
 *  В SettingsScreen.cpp их БЫТЬ НЕ ДОЛЖНО.
 */

// маленький helper
static int clampI(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// ============================================================================
// LEFT
// ============================================================================
void SettingsScreen::navLeft() {

    switch (_level) {

        // ------------------------------------------------------------
        // ROOT menu
        // ------------------------------------------------------------
        case Level::ROOT: {
            const int n = sizeof(MENU) / sizeof(MENU[0]);
            _selected = (_selected == 0) ? (n - 1) : (_selected - 1);
            break;
        }

        // ------------------------------------------------------------
        // WIFI main menu (State / Scan)
        // ------------------------------------------------------------
        case Level::WIFI: {
            _subSelected = clampI(_subSelected - 1, 0, 1);
            break;
        }

        // ------------------------------------------------------------
        // WIFI LIST (SSID + Rescan)
        // ------------------------------------------------------------
        case Level::WIFI_LIST: {

            int count = _wifi.networksCount();

            // только Rescan
            if (count == 0) {
                _wifiListSelected = 0;
                _wifiListTop = 0;
                break;
            }

            int maxIdx = count; // + Rescan
            _wifiListSelected = clampI(_wifiListSelected - 1, 0, maxIdx);

            constexpr int VISIBLE_ROWS = 4;

            if (_wifiListSelected < _wifiListTop)
                _wifiListTop = _wifiListSelected;

            break;
        }

        // ------------------------------------------------------------
        // Остальные экраны пока не трогаем
        // ------------------------------------------------------------
        default:
            break;
    }

    _dirty = true;
}

// ============================================================================
// RIGHT
// ============================================================================
void SettingsScreen::navRight() {

    switch (_level) {

        // ------------------------------------------------------------
        // ROOT menu
        // ------------------------------------------------------------
        case Level::ROOT: {
            const int n = sizeof(MENU) / sizeof(MENU[0]);
            _selected = (_selected + 1) % n;
            break;
        }

        // ------------------------------------------------------------
        // WIFI main menu
        // ------------------------------------------------------------
        case Level::WIFI: {
            _subSelected = clampI(_subSelected + 1, 0, 1);
            break;
        }

        // ------------------------------------------------------------
        // WIFI LIST
        // ------------------------------------------------------------
        case Level::WIFI_LIST: {

            int count = _wifi.networksCount();

            if (count == 0) {
                _wifiListSelected = 0;
                _wifiListTop = 0;
                break;
            }

            int maxIdx = count; // + Rescan
            _wifiListSelected = clampI(_wifiListSelected + 1, 0, maxIdx);

            constexpr int VISIBLE_ROWS = 4;

            if (_wifiListSelected >= _wifiListTop + VISIBLE_ROWS)
                _wifiListTop = _wifiListSelected - (VISIBLE_ROWS - 1);

            break;
        }

        default:
            break;
    }

    _dirty = true;
}