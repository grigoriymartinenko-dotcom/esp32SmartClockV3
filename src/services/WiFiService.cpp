#include "services/WifiService.h"

#include <Arduino.h>
#include <algorithm>
#include <cstring>

/*
 * WifiService.cpp
 * ----------------
 * ЕДИНЫЙ источник истины о Wi-Fi.
 *
 * Что гарантирует этот файл:
 *
 * 1. Если WiFi.status() == WL_CONNECTED
 *    → в списке сетей ОБЯЗАТЕЛЬНО есть ровно ОДНА сеть с connected = true
 *
 * 2. Подключённая сеть ВСЕГДА вверху списка
 *
 * 3. Scan:
 *    - полностью пересобирает список сетей
 *    - НО НЕ теряет подключённую сеть
 *
 * 4. UI НЕ мигает:
 *    - UiVersion bump происходит ТОЛЬКО при реальных изменениях
 *
 * 5. Полная совместимость с существующим SettingsWifi.cpp
 *    (ssidAt(), connect(), connect(ssid, pass) — всё на месте)
 */

// ============================================================================
// helpers (локальные утилиты, UI их НЕ видит)
// ============================================================================

static void copySsid(char out[33], const char* in) {
    if (!in) {
        out[0] = 0;
        return;
    }
    strncpy(out, in, 32);
    out[32] = 0;
}

static bool ssidEquals(const char* a, const char* b) {
    return a && b && strcmp(a, b) == 0;
}

/*
 * Сортировка сетей:
 *  1) connected — всегда выше
 *  2) дальше по RSSI (больше — лучше)
 */
static bool netLess(const WifiService::Network& a,
                    const WifiService::Network& b) {

    if (a.connected != b.connected)
        return a.connected > b.connected;

    return a.rssi > b.rssi;
}

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
// UI version helpers
// ============================================================================

void WifiService::bumpList() {
    _listVersion++;
    _ui.bump(UiChannel::WIFI);
}

void WifiService::bumpState() {
    _stateVersion++;
    _ui.bump(UiChannel::WIFI);
}

void WifiService::recomputeConnectedIndex() {
    _connectedIndex = -1;
    for (int i = 0; i < (int)_networks.size(); i++) {
        if (_networks[i].connected) {
            _connectedIndex = i;
            return;
        }
    }
}

// ============================================================================
// begin
// ============================================================================
void WifiService::begin() {

    _enabled = _prefs.wifiEnabled();

    bumpState();
    bumpList();

    if (_enabled)
        start();
    else
        stop();
}

// ============================================================================
// update (ГЛАВНАЯ МАШИНА СОСТОЯНИЙ)
// ============================================================================
void WifiService::update() {

    if (!_enabled)
        return;

    // ------------------------------------------------------------------------
    // READ REAL WIFI STATE
    // ------------------------------------------------------------------------
    wl_status_t st = WiFi.status();
    bool nowConnected = (st == WL_CONNECTED);

    // ------------------------------------------------------------------------
    // STATE TRANSITIONS
    // ------------------------------------------------------------------------
    if (nowConnected && _state != State::ONLINE) {
        _state = State::ONLINE;
        copySsid(_currentSsid, WiFi.SSID().c_str());
        bumpState();
    }

    if (!nowConnected && _state == State::ONLINE) {
        _state = State::ERROR;
        _currentSsid[0] = 0;
        bumpState();
    }

    // ------------------------------------------------------------------------
    // CONNECT TIMEOUT
    // ------------------------------------------------------------------------
    if (_state == State::CONNECTING) {
        if (millis() - _connectStartMs > CONNECT_TIMEOUT_MS) {
            _state = State::ERROR;
            WiFi.disconnect(true);
            bumpState();
        }
    }

    // ------------------------------------------------------------------------
    // SCAN STATE MACHINE
    // ------------------------------------------------------------------------
    if (_scanState == ScanState::SCANNING) {

        int res = WiFi.scanComplete();
        if (res == WIFI_SCAN_RUNNING)
            return;

        // ВАЖНО:
        // Scan всегда ПЕРЕСОБИРАЕТ список сетей с нуля
        _networks.clear();

        if (res == WIFI_SCAN_FAILED) {
            _scanState = ScanState::FAILED;
            bumpState();
            bumpList();
            return;
        }

        _scanState = ScanState::DONE;

        for (int i = 0; i < res; i++) {

            Network n{};
            copySsid(n.ssid, WiFi.SSID(i).c_str());

            n.rssi    = (int16_t)WiFi.RSSI(i);
            n.secured = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
            n.saved   = _prefs.hasWifiCredentials() &&
                        ssidEquals(_prefs.wifiSsid(), n.ssid);
            n.connected = false; // connected добавляется ПОЗЖЕ

            _networks.push_back(n);
        }

        WiFi.scanDelete();
        bumpList();
        bumpState();
    }

    // ------------------------------------------------------------------------
    // ENSURE CONNECTED NETWORK EXISTS (КЛЮЧЕВОЙ БЛОК)
    // ------------------------------------------------------------------------
    if (_state == State::ONLINE && _currentSsid[0]) {

        bool found   = false;
        bool changed = false;

        for (auto& n : _networks) {
            if (ssidEquals(n.ssid, _currentSsid)) {
                if (!n.connected) {
                    n.connected = true;
                    changed = true;
                }
                n.rssi = (int16_t)WiFi.RSSI();
                found = true;
            } else {
                if (n.connected) {
                    n.connected = false;
                    changed = true;
                }
            }
        }

        // Если подключённой сети нет в scan → ДОБАВЛЯЕМ
        if (!found) {
            Network n{};
            copySsid(n.ssid, _currentSsid);
            n.connected = true;
            n.secured   = true;
            n.saved     = true;
            n.rssi      = (int16_t)WiFi.RSSI();
            _networks.push_back(n);
            changed = true;
        }

        // ВАЖНО:
        // bumpList() ТОЛЬКО если реально что-то изменилось
        if (changed) {
            std::stable_sort(_networks.begin(), _networks.end(), netLess);
            recomputeConnectedIndex();
            bumpList();
        }
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
        _networks.clear();
        _connectedIndex = -1;
        _currentSsid[0] = 0;
        stop();
    }

    bumpState();
    bumpList();
}

bool WifiService::isEnabled() const {
    return _enabled;
}

WifiService::State WifiService::state() const {
    return _state;
}

bool WifiService::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
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
        bumpState();
        return;
    }

    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.begin();

    _connectStartMs = millis();
    _state = State::CONNECTING;
    bumpState();
}

void WifiService::stop() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    _state = State::OFF;
    bumpState();
}

// ============================================================================
// CONNECT (ОБЕ ПЕРЕГРУЗКИ — ОБЯЗАТЕЛЬНЫ)
// ============================================================================
void WifiService::connect(const char* ssid) {

    if (!_enabled || !ssid || !ssid[0])
        return;

    const char* pass = nullptr;
    if (_prefs.hasWifiCredentials() &&
        ssidEquals(_prefs.wifiSsid(), ssid))
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
    bumpState();
}

void WifiService::connect(const char* ssid, const char* pass) {

    if (!_enabled || !ssid || !ssid[0])
        return;

    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
    WiFi.begin(ssid, pass);

    _prefs.setWifiCredentials(ssid, pass ? pass : "");
    _prefs.save();

    _connectStartMs = millis();
    _state = State::CONNECTING;
    bumpState();
}

// ============================================================================
// SCAN
// ============================================================================
void WifiService::startScan() {

    if (!_enabled)
        return;

    if (_scanState == ScanState::SCANNING)
        return;

    WiFi.scanDelete();
    WiFi.scanNetworks(true, false, true); // async

    _scanState = ScanState::SCANNING;
    bumpState();
}

// ============================================================================
// GETTERS FOR UI
// ============================================================================
WifiService::ScanState WifiService::scanState() const {
    return _scanState;
}

int WifiService::networksCount() const {
    return (int)_networks.size();
}

const WifiService::Network& WifiService::networkAt(int i) const {
    static Network dummy{};
    if (i < 0 || i >= (int)_networks.size())
        return dummy;
    return _networks[i];
}


uint32_t WifiService::listVersion() const {
    return _listVersion;
}

uint32_t WifiService::stateVersion() const {
    return _stateVersion;
}

const char* WifiService::currentSsid() const {
    return (_state == State::ONLINE && _currentSsid[0])
        ? _currentSsid
        : nullptr;
}

int WifiService::connectedIndex() const {
    return _connectedIndex;
}