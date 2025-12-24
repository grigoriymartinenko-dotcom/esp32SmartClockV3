#include "screens/SettingsScreen.h"

/*
 * SettingsNav.cpp
 * ----------------
 * Навигация по меню Settings.
 *
 * ПРАВИЛА:
 *  - НИКАКОЙ отрисовки
 *  - ТОЛЬКО изменение индексов
 *  - Scroll-логика живёт ЗДЕСЬ, а не в draw
 */

void SettingsScreen::navLeft() {

    // ===== ROOT =====
    if (_level == Level::ROOT) {
        if (_selected > 0) _selected--;
        _dirty = true;
        return;
    }

    // ===== WIFI MENU =====
    if (_level == Level::WIFI) {
        if (_subSelected > 0) _subSelected--;
        _dirty = true;
        return;
    }

    // ===== WIFI LIST =====
    if (_level == Level::WIFI_LIST) {

        int total = _wifi.networksCount(); // ТОЛЬКО SSID

        if (_wifiListSelected > 0)
            _wifiListSelected--;

        // --- SCROLL UP ---
        if (_wifiListSelected < _wifiListTop)
            _wifiListTop = _wifiListSelected;

        _dirty = true;
        return;
    }
}

void SettingsScreen::navRight() {

    // ===== ROOT =====
    if (_level == Level::ROOT) {
        int max = (int)(sizeof(MENU) / sizeof(MENU[0])) - 1;
        if (_selected < max) _selected++;
        _dirty = true;
        return;
    }

    // ===== WIFI MENU =====
    if (_level == Level::WIFI) {
        if (_subSelected < 1) _subSelected++;
        _dirty = true;
        return;
    }

    // ===== WIFI LIST =====
    if (_level == Level::WIFI_LIST) {

        int total =
            _wifi.networksCount() + 1; // + Rescan

        if (_wifiListSelected < total - 1)
            _wifiListSelected++;

        // --- SCROLL DOWN ---
        constexpr int VISIBLE_ROWS = 4;

        if (_wifiListSelected >= _wifiListTop + VISIBLE_ROWS)
            _wifiListTop = _wifiListSelected - VISIBLE_ROWS + 1;

        _dirty = true;
        return;
    }
}