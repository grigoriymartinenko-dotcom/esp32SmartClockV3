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
 *  - ASYNC scan сетей (с явным lifecycle)
 *  - CONNECT к выбранному SSID
 *
 * ВАЖНО:
 *  WifiService НЕ знает про UI.
 *  Он только меняет State / ScanState
 *  и делает ui.bump(UiChannel::WIFI).
 */

class WifiService {
public:
    enum class State {
        OFF,
        CONNECTING,
        ONLINE,
        ERROR
    };

    enum class ScanState {
        IDLE,
        SCANNING,
        DONE,
        FAILED
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
    State state() const;

    // текущий SSID или nullptr если не подключены
    const char* currentSsid() const;

    // ===== CONNECT =====
    void connect(const char* ssid);
    void connect(const char* ssid, const char* pass);

    // ===== SCAN =====
    void startScan();

    ScanState scanState() const;
    int  networksCount() const;
    const char* ssidAt(int i) const;

private:
    void start();
    void stop();

    // ===== deps =====
    UiVersionService&    _ui;
    PreferencesService& _prefs;

    // ===== wifi state =====
    State _state = State::OFF;
    bool  _enabled = false;

    // ===== connect timeout =====
    unsigned long _connectStartMs = 0;
    static constexpr unsigned long CONNECT_TIMEOUT_MS = 15000;

    // ===== scan state =====
    ScanState _scanState = ScanState::IDLE;
    int       _scanCount = 0;

    // ===== scan cache =====
    std::vector<String> _ssids;
};