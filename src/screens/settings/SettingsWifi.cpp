#include "screens/SettingsScreen.h"
#include <cstring>

extern PreferencesService prefs;

// ============================================================================
// WIFI: Short OK
// ============================================================================
bool SettingsScreen::handleWifiShortOk() {

    // ------------------------------------------------------------------------
    // LEVEL: WIFI
    // ------------------------------------------------------------------------
    if (_level == Level::WIFI) {

        if (_subSelected == 1) {
            _wifi.startScan();
            _wifiListSelected = 0;
            _wifiListTop      = 0;
            enterSubmenu(Level::WIFI_LIST);
        }

        return true;
    }

    // ------------------------------------------------------------------------
    // LEVEL: WIFI_LIST
    // ------------------------------------------------------------------------
    if (_level == Level::WIFI_LIST) {

        if (_wifi.scanState() == WifiService::ScanState::SCANNING)
            return true;

        const int netCount = _wifi.networksCount();
        const bool isRescan = (_wifiListSelected == netCount);

        if (netCount <= 0 || isRescan) {
            _wifi.startScan();
            return true;
        }

        memset(_wifiPass, 0, sizeof(_wifiPass));
        _wifiPassLen = 0;
        _wifiCharIdx = 0;

        enterSubmenu(Level::WIFI_PASSWORD);
        return true;
    }

    // ------------------------------------------------------------------------
    // LEVEL: WIFI_PASSWORD
    // ------------------------------------------------------------------------
    if (_level == Level::WIFI_PASSWORD) {

        if (_wifiPassLen < WIFI_PASS_MAX) {
            _wifiPass[_wifiPassLen++] = PASS_CHARS[_wifiCharIdx];
            _wifiPass[_wifiPassLen]   = 0;
        }
        return true;
    }

    return false;
}

// ============================================================================
// WIFI: Short BACK
// ============================================================================
bool SettingsScreen::handleWifiShortBack() {

    if (_level != Level::WIFI_PASSWORD)
        return false;

    if (_wifiPassLen > 0) {
        _wifiPass[--_wifiPassLen] = 0;
        return true;
    }

    enterSubmenu(Level::WIFI_LIST);
    return true;
}

// ============================================================================
// WIFI: Long OK
// ============================================================================
bool SettingsScreen::handleWifiLongOk() {

    if (_level != Level::WIFI_PASSWORD)
        return false;

    const auto& net = _wifi.networkAt(_wifiListSelected);

    if (!net.ssid[0])
        return true;

    prefs.setWifiCredentials(net.ssid, _wifiPass);
    prefs.save();

    _wifi.connect(net.ssid, _wifiPass);
    enterSubmenu(Level::WIFI_LIST);
    return true;
}

// ============================================================================
// WIFI: Long BACK
// ============================================================================
bool SettingsScreen::handleWifiLongBack() {

    if (_level != Level::WIFI_PASSWORD)
        return false;

    enterSubmenu(Level::WIFI_LIST);
    return true;
}