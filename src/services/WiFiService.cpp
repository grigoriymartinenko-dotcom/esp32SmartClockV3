#include "services/WifiService.h"

#include <Arduino.h>
#include <cstring>

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
 *
 * ВАЖНО ДЛЯ UI:
 *  - UI хочет пометить сеть в списке как [Connected]
 *  - Для этого ему нужен SSID текущего соединения
 *
 * РЕШЕНИЕ:
 *  - currentSsid() возвращает SSID из PreferencesService,
 *    который мы:
 *      1) записываем как "intent" при connect(ssid, pass)
 *      2) подтверждаем фактическим SSID при WL_CONNECTED (WiFi.SSID())
 *
 * Таким образом:
 *  - пометка [Connected] появляется ТОЛЬКО когда state ONLINE
 *  - SSID всегда валиден и одинаков для UI и сервиса
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
    // Wi-Fi OFF → НИЧЕГО не делаем, включая scan
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

            // ✅ КРИТИЧНО:
            // Зафиксировать актуальный SSID в Preferences, чтобы UI мог
            // пометить [Connected] рядом с SSID из scan-листа.
            //
            // WiFi.SSID() — источник правды "к чему реально подключились".
            String cur = WiFi.SSID();
            if (cur.length() > 0) {
                const char* pass = _prefs.wifiPass(); // мог быть записан раньше при connect(ssid, pass)
                _prefs.setWifiCredentials(cur.c_str(), pass ? pass : "");
                _prefs.save();
            }

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
        // OFF → принудительно останавливаем scan
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

    // Если в prefs есть креды — подключаемся явно (предсказуемо для UI)
    if (_prefs.hasWifiCredentials() && _prefs.wifiSsid()[0]) {

        WiFi.setAutoConnect(false);
        WiFi.setAutoReconnect(false);

        const char* ssid = _prefs.wifiSsid();
        const char* pass = _prefs.wifiPass();

        if (pass && pass[0]) {
            WiFi.begin(ssid, pass);
        } else {
            WiFi.begin(ssid);
        }

        _connectStartMs = millis();
        _state = State::CONNECTING;
        _ui.bump(UiChannel::WIFI);
        return;
    }

    // Иначе — как было: begin() без аргументов (если стек WiFi помнит сеть)
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

    Serial.printf("[WiFi] connect to '%s' (open or saved pass)\n", ssid);

    // Если пароль для этого SSID уже сохранён — используем его
    const char* pass = nullptr;
    if (_prefs.hasWifiCredentials() && strcmp(_prefs.wifiSsid(), ssid) == 0) {
        pass = _prefs.wifiPass();
    }

    if (pass && pass[0]) {
        connect(ssid, pass);
        return;
    }

    // Open network / no password
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    WiFi.begin(ssid);

    // ✅ Для UI: сохраняем intent (SSID), но пометка появится только когда ONLINE
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

    // ✅ Для UI: сохраняем intent (SSID/PASS), но пометка появится только когда ONLINE
    _prefs.setWifiCredentials(ssid, pass ? pass : "");
    _prefs.save();

    _connectStartMs = millis();
    _state = State::CONNECTING;
    _ui.bump(UiChannel::WIFI);
}

// ============================================================================
// SCAN (ASYNC, PASSIVE)
// ============================================================================
void WifiService::startScan() {

    // НЕЛЬЗЯ сканировать, если Wi-Fi выключен
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