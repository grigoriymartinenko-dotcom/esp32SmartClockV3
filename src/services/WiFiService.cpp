#include "services/WifiService.h"

#include <Arduino.h>
#include <cstring>

/*
 * WifiService.cpp
 * ----------------
 * Управляет Wi-Fi:
 *  - ON / OFF
 *  - CONNECTING / ONLINE / ERROR
 *  - ASYNC scan сетей (явный lifecycle)
 *  - CONNECT к выбранному SSID
 *
 * ИНВАРИАНТ:
 *  Wi-Fi OFF ⇒ НИКАКИХ scan / connect
 */

// ============================================================================
// ctor
// ============================================================================
WifiService::WifiService(
    UiVersionService& ui,
    PreferencesService& prefs
)
    : _ui(ui)
    , _prefs(prefs)
{}

// ============================================================================
// begin
// ============================================================================
void WifiService::begin() {
    _enabled = _prefs.wifiEnabled();
    if (_enabled)
        start();
    else
        stop();
}

// ============================================================================
// update
// ============================================================================
void WifiService::update() {

    // ------------------------------------------------------------------------
    // Wi-Fi OFF → ничего не делаем
    // ------------------------------------------------------------------------
    if (!_enabled)
        return;

    // ------------------------------------------------------------------------
    // CONNECT STATE
    // ------------------------------------------------------------------------
    wl_status_t st = WiFi.status();

    if (st == WL_CONNECTED) {
        if (_state != State::ONLINE) {
            _state = State::ONLINE;

            // Зафиксировать фактический SSID
            String cur = WiFi.SSID();
            if (cur.length() > 0) {
                const char* pass = _prefs.wifiPass();
                _prefs.setWifiCredentials(cur.c_str(), pass ? pass : "");
                _prefs.save();
            }

            _ui.bump(UiChannel::WIFI);
        }
    }
    else if (_state == State::CONNECTING) {
        if (millis() - _connectStartMs > CONNECT_TIMEOUT_MS) {
            _state = State::ERROR;
            WiFi.disconnect(true);
            _ui.bump(UiChannel::WIFI);
        }
    }

    // ------------------------------------------------------------------------
    // SCAN STATE MACHINE
    // ------------------------------------------------------------------------
    if (_scanState == ScanState::SCANNING) {

        int res = WiFi.scanComplete();

        if (res == WIFI_SCAN_RUNNING)
            return;

        _ssids.clear();

        if (res == WIFI_SCAN_FAILED) {
            Serial.println("[WiFi] scan failed");
            _scanCount = 0;
            _scanState = ScanState::FAILED;
            _ui.bump(UiChannel::WIFI);
            return;
        }

        _scanCount = res;
        _scanState = ScanState::DONE;

        _ssids.reserve(res);
        for (int i = 0; i < res; i++) {
            _ssids.push_back(WiFi.SSID(i));
        }

        WiFi.scanDelete();
        _ui.bump(UiChannel::WIFI);
    }
}

// ============================================================================
// ENABLE / DISABLE
// ============================================================================
void WifiService::setEnabled(bool on) {

    if (_enabled == on)
        return;

    _enabled = on;
    _prefs.setWifiEnabled(on);
    _prefs.save();

    if (on) {
        start();
    } else {
        WiFi.scanDelete();
        _scanState = ScanState::IDLE;
        _scanCount = 0;
        _ssids.clear();
        stop();
    }
}

bool WifiService::isEnabled() const {
    return _enabled;
}

WifiService::State WifiService::state() const {
    return _state;
}

// ============================================================================
// START / STOP
// ============================================================================
void WifiService::start() {

    WiFi.mode(WIFI_STA);

    if (_prefs.hasWifiCredentials() && _prefs.wifiSsid()[0]) {

        WiFi.setAutoConnect(false);
        WiFi.setAutoReconnect(false);

        const char* ssid = _prefs.wifiSsid();
        const char* pass = _prefs.wifiPass();

        if (pass && pass[0])
            WiFi.begin(ssid, pass);
        else
            WiFi.begin(ssid);

        _connectStartMs = millis();
        _state = State::CONNECTING;
        _ui.bump(UiChannel::WIFI);
        return;
    }

    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.begin();

    _connectStartMs = millis();
    _state = State::CONNECTING;
    _ui.bump(UiChannel::WIFI);
}

void WifiService::stop() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    _state = State::OFF;
    _ui.bump(UiChannel::WIFI);
}

// ============================================================================
// CONNECT
// ============================================================================
void WifiService::connect(const char* ssid) {

    if (!_enabled || !ssid || !ssid[0])
        return;

    Serial.printf("[WiFi] connect to '%s'\n", ssid);

    const char* pass = nullptr;
    if (_prefs.hasWifiCredentials() && strcmp(_prefs.wifiSsid(), ssid) == 0)
        pass = _prefs.wifiPass();

    if (pass && pass[0]) {
        connect(ssid, pass);
        return;
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    WiFi.begin(ssid);

    _prefs.setWifiCredentials(ssid, "");
    _prefs.save();

    _connectStartMs = millis();
    _state = State::CONNECTING;
    _ui.bump(UiChannel::WIFI);
}

void WifiService::connect(const char* ssid, const char* pass) {

    if (!_enabled || !ssid || !ssid[0])
        return;

    Serial.printf("[WiFi] connect to '%s' with password\n", ssid);

    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    WiFi.begin(ssid, pass);

    _prefs.setWifiCredentials(ssid, pass ? pass : "");
    _prefs.save();

    _connectStartMs = millis();
    _state = State::CONNECTING;
    _ui.bump(UiChannel::WIFI);
}

// ============================================================================
// SCAN
// ============================================================================
void WifiService::startScan() {

    if (!_enabled)
        return;

    if (_scanState == ScanState::SCANNING)
        return;

    Serial.println("[WiFi] start async scan");

    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);

    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    WiFi.scanDelete();
    WiFi.scanNetworks(true, false, true); // async + hidden + passive

    _scanState = ScanState::SCANNING;
    _scanCount = 0;
    _ssids.clear();

    _ui.bump(UiChannel::WIFI);
}

// ============================================================================
// SCAN INFO
// ============================================================================
WifiService::ScanState WifiService::scanState() const {
    return _scanState;
}

int WifiService::networksCount() const {
    return _scanCount;
}

const char* WifiService::ssidAt(int i) const {
    if (i < 0 || i >= (int)_ssids.size())
        return "";
    return _ssids[i].c_str();
}

// ============================================================================
// STATUS (for UI)
// ============================================================================
const char* WifiService::currentSsid() const {
    if (_state != State::ONLINE)
        return nullptr;

    const char* s = _prefs.wifiSsid();
    if (!s || !s[0])
        return nullptr;

    return s;
}