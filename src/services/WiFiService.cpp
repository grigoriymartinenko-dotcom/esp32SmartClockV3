#include "services/WifiService.h"

/*
 * WifiService.cpp
 * ----------------
 * Управляет Wi-Fi:
 *  - ON / OFF
 *  - CONNECTING / ONLINE / ERROR
 *  - ASYNC scan сетей
 *
 * КРИТИЧЕСКИ ВАЖНО:
 *  ESP32 НЕ сканирует эфир корректно,
 *  если STA уже был активен.
 *
 *  Единственный надёжный способ:
 *   - WIFI_OFF
 *   - WIFI_STA
 *   - PASSIVE scan
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
    if (_enabled) start();
    else          stop();
}

// ============================================================================
// update
// ============================================================================
void WifiService::update() {

    if (!_enabled)
        return;

    wl_status_t st = WiFi.status();

    if (st == WL_CONNECTED) {
        if (_state != State::ONLINE) {
            _state = State::ONLINE;
            _ui.bump(UiChannel::WIFI);
        }
    }
    else if (_state == State::CONNECTING) {
        if (millis() - _connectStartMs > CONNECT_TIMEOUT_MS) {
            _state = State::ERROR;
            _ui.bump(UiChannel::WIFI);
        }
    }

    if (_scanInProgress) {

        int res = WiFi.scanComplete();

        if (res == WIFI_SCAN_RUNNING)
            return;

        _scanInProgress = false;
        _scanFinished  = true;
        _ssids.clear();

        if (res == WIFI_SCAN_FAILED) {
            Serial.println("[WiFi] scan failed");
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

        Serial.printf("[WiFi] scan finished: %d networks\n", res);
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

    on ? start() : stop();
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
// SCAN (ASYNC, PASSIVE) — ФИНАЛ
// ============================================================================
void WifiService::startScan() {

    if (_scanInProgress)
        return;

    Serial.println("[WiFi] start async scan");

    // ❗ ПОЛНЫЙ RESET STA
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);

    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    WiFi.scanDelete();

    // async = true
    // showHidden = true
    // passive = true  ← КЛЮЧ
    WiFi.scanNetworks(true, false, true);

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