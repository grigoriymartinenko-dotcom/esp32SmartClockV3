#include "services/WifiService.h"

/*
 * WifiService.cpp
 * ----------------
 * Управляет Wi-Fi:
 *  - ON / OFF
 *  - CONNECTING / ONLINE / ERROR
 *  - ASYNC scan сетей
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
    // 🔥 FIX: Wi-Fi OFF → НИЧЕГО не делаем, включая scan
    // ------------------------------------------------------------------------
    if (!_enabled)
        return;

    wl_status_t st = WiFi.status();

    // ------------------------------------------------------------------------
    // CONNECTED → ONLINE
    // ------------------------------------------------------------------------
    if (st == WL_CONNECTED) {
        if (_state != State::ONLINE) {
            _state = State::ONLINE;
            _ui.bump(UiChannel::WIFI);
        }
        return;
    }

    // ------------------------------------------------------------------------
    // CONNECTING → timeout → ERROR
    // ------------------------------------------------------------------------
    if (_state == State::CONNECTING) {
        if (millis() - _connectStartMs > CONNECT_TIMEOUT_MS) {
            _state = State::ERROR;
            WiFi.disconnect(true);
            _ui.bump(UiChannel::WIFI);
        }
    }

    // ------------------------------------------------------------------------
    // ASYNC SCAN RESULT
    // ------------------------------------------------------------------------
    if (_scanInProgress) {

        int res = WiFi.scanComplete();

        if (res == WIFI_SCAN_RUNNING)
            return;

        _scanInProgress = false;
        _scanFinished  = true;
        _ssids.clear();

        if (res == WIFI_SCAN_FAILED) {
            _scanCount = 0;
            _ui.bump(UiChannel::WIFI);
            return;
        }

        _scanCount = res;
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

        // --------------------------------------------------------------------
        // 🔥 FIX: ПРИНУДИТЕЛЬНО останавливаем scan при OFF
        // --------------------------------------------------------------------
        if (_scanInProgress) {
            WiFi.scanDelete();
            _scanInProgress = false;
            _scanFinished  = false;
            _scanCount = 0;
            _ssids.clear();
        }

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
// CONNECT TO SELECTED SSID
// ============================================================================
void WifiService::connect(const char* ssid) {

    if (!_enabled)
        return;

    if (!ssid || !ssid[0])
        return;

    Serial.printf("[WiFi] connect to '%s'\n", ssid);

    // гарантируем чистое подключение
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    WiFi.begin(ssid);

    _connectStartMs = millis();
    _state = State::CONNECTING;
    _ui.bump(UiChannel::WIFI);
}

// ============================================================================
// SCAN (ASYNC, PASSIVE)
// ============================================================================
void WifiService::startScan() {

    // ------------------------------------------------------------------------
    // 🔥 FIX: НЕЛЬЗЯ сканировать, если Wi-Fi выключен
    // ------------------------------------------------------------------------
    if (!_enabled)
        return;

    if (_scanInProgress)
        return;

    Serial.println("[WiFi] start async scan");

    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);

    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    WiFi.scanDelete();
    WiFi.scanNetworks(true, false, true); // async + hidden + passive

    _scanInProgress = true;
    _scanFinished  = false;
    _scanCount = 0;
    _ssids.clear();

    _ui.bump(UiChannel::WIFI);
}

// ============================================================================
// SCAN INFO
// ============================================================================
bool WifiService::isScanning() const {
    return _scanInProgress;
}

bool WifiService::isScanFinished() const {
    return _scanFinished;
}

int WifiService::networksCount() const {
    return _scanCount;
}

const char* WifiService::ssidAt(int i) const {
    if (i < 0 || i >= (int)_ssids.size())
        return "";
    return _ssids[i].c_str();
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

    _connectStartMs = millis();
    _state = State::CONNECTING;
    _ui.bump(UiChannel::WIFI);
}