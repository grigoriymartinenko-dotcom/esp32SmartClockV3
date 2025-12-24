#include "screens/SettingsScreen.h"
#include <cstring>

extern PreferencesService prefs;

/*
 * SettingsWifi.cpp
 * ----------------
 * Вся логика Wi-Fi:
 *  - меню Wi-Fi
 *  - список сетей
 *  - ввод пароля
 *  - сохранение + connect
 *
 * НИКАКОЙ отрисовки.
 */

// ============================================================================
// WIFI: Short OK
// ============================================================================
bool SettingsScreen::handleWifiShortOk() {

    // ------------------------------------------------------------------------
    // WIFI → Scan → WIFI_LIST
    // ------------------------------------------------------------------------
    if (_level == Level::WIFI) {

        // 0 = State (пока не редактируем)
        // 1 = Scan
        if (_subSelected == 1) {
            _wifi.startScan();
            _wifiListSelected = 0;
            _wifiListTop      = 0;
            enterSubmenu(Level::WIFI_LIST);
            return true;
        }

        _dirty = true;
        return true;
    }

    // ------------------------------------------------------------------------
    // WIFI_LIST → выбор SSID → ввод пароля
    // ------------------------------------------------------------------------
    if (_level == Level::WIFI_LIST) {

        if (_wifi.isScanning()) {
            _dirty = true;
            return true;
        }

        const int netCount = _wifi.networksCount();

        // пусто → только Rescan
        if (netCount <= 0) {
            _wifi.startScan();
            _dirty = true;
            return true;
        }

        // Rescan
        if (_wifiListSelected == netCount) {
            _wifi.startScan();
            _dirty = true;
            return true;
        }

        // SSID выбран → ввод пароля
        memset(_wifiPass, 0, sizeof(_wifiPass));
        _wifiPassLen = 0;
        _wifiCharIdx = 0;

        enterSubmenu(Level::WIFI_PASSWORD);
        return true;
    }

    // ------------------------------------------------------------------------
    // WIFI_PASSWORD → добавить символ
    // ------------------------------------------------------------------------
    if (_level == Level::WIFI_PASSWORD) {

        if (_wifiPassLen < WIFI_PASS_MAX) {
            _wifiPass[_wifiPassLen++] = PASS_CHARS[_wifiCharIdx];
            _wifiPass[_wifiPassLen]   = 0;
            _dirty = true;
        }
        return true;
    }

    return false;
}

// ============================================================================
// WIFI: Short BACK
// ============================================================================
bool SettingsScreen::handleWifiShortBack() {

    if (_level == Level::WIFI_PASSWORD) {

        // Backspace
        if (_wifiPassLen > 0) {
            _wifiPass[--_wifiPassLen] = 0;
            _dirty = true;
            return true;
        }

        // выйти к списку
        enterSubmenu(Level::WIFI_LIST);
        return true;
    }

    return false;
}

// ============================================================================
// WIFI: Long OK (APPLY)
// ============================================================================
bool SettingsScreen::handleWifiLongOk() {

    if (_level != Level::WIFI_PASSWORD)
        return false;

    const char* ssid = _wifi.ssidAt(_wifiListSelected);

    if (!ssid || !ssid[0]) {
        _dirty = true;
        return true;
    }

    // 1) save
    prefs.setWifiCredentials(ssid, _wifiPass);
    prefs.save();

    // 2) connect
    _wifi.connect(ssid, _wifiPass);

    // 3) назад к списку (там увидим статус)
    enterSubmenu(Level::WIFI_LIST);
    return true;
}

// ============================================================================
// WIFI: Long BACK (CANCEL)
// ============================================================================
bool SettingsScreen::handleWifiLongBack() {

    if (_level == Level::WIFI_PASSWORD) {
        enterSubmenu(Level::WIFI_LIST);
        return true;
    }

    return false;
}