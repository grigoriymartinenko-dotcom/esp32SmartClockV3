#pragma once
#include <WiFi.h>
#include <vector>
#include <string>

#include "services/UiVersionService.h"
#include "services/PreferencesService.h"

/*
 * WifiService
 * -----------
 * Управляет Wi-Fi:
 *  - ON / OFF
 *  - CONNECTING / ONLINE / ERROR
 *  - ASYNC scan сетей
 *  - CONNECT к выбранному SSID
 *
 * ВАЖНО:
 *  WifiService НЕ знает про UI.
 *  Он только меняет State и делает ui.bump(WIFI).
 */

class WifiService {
public:
    enum class State {
        OFF,
        CONNECTING,
        ONLINE,
        ERROR
    };

    WifiService(
        UiVersionService& ui,
        PreferencesService& prefs
    );

    // lifecycle
    void begin();
    void update();

    // ON / OFF
    void setEnabled(bool on);
    bool isEnabled() const;

    // ===== STATUS =====
// текущий SSID или nullptr если не подключены
const char* currentSsid() const;


    // состояние подключения
    State state() const;

    // ===== CONNECT =====
    void connect(const char* ssid);
    void connect(const char* ssid, const char* pass); // 🔥 НОВОЕ

    // ===== SCAN =====
    void startScan();
    bool isScanning() const;
    bool isScanFinished() const;

    int  networksCount() const;
    const char* ssidAt(int i) const;

private:
    void start();
    void stop();

    // ===== scan state =====
    bool _scanInProgress = false;
    bool _scanFinished  = false;
    int  _scanCount     = 0;

    // ===== deps =====
    UiVersionService&    _ui;
    PreferencesService& _prefs;

    // ===== wifi state =====
    State _state = State::OFF;
    bool  _enabled = false;

    // ===== connect timeout =====
    unsigned long _connectStartMs = 0;
    static constexpr unsigned long CONNECT_TIMEOUT_MS = 15000;

    // ===== scan cache =====
    std::vector<String> _ssids;
};