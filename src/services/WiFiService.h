#pragma once

#include <WiFi.h>
#include <cstdint>
#include <vector>

#include "services/UiVersionService.h"
#include "services/PreferencesService.h"

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

    static constexpr int16_t RSSI_UNKNOWN = INT16_MIN;

    struct Network {
        char    ssid[33];
        int16_t rssi;
        bool    secured;
        bool    connected;
        bool    saved;
    };

    WifiService(
        UiVersionService& ui,
        PreferencesService& prefs
    );

    void begin();
    void update();

    void setEnabled(bool on);
    bool isEnabled() const;

    State state() const;
    bool isConnected() const;
    const char* currentSsid() const;
    int connectedIndex() const;

    void connect(const char* ssid);
    void connect(const char* ssid, const char* pass);
    void disconnect();

    void startScan();
    ScanState scanState() const;

    int networksCount() const;
    const Network& networkAt(int i) const;

    uint32_t listVersion() const;
    uint32_t stateVersion() const;

private:
    void start();
    void stop();

    void bumpList();
    void bumpState();
    void recomputeConnectedIndex();

    UiVersionService&    _ui;
    PreferencesService& _prefs;

    State _state = State::OFF;
    bool  _enabled = false;

    unsigned long _connectStartMs = 0;
    static constexpr unsigned long CONNECT_TIMEOUT_MS = 15000;

    ScanState _scanState = ScanState::IDLE;

    std::vector<Network> _networks;
    int _connectedIndex = -1;

    char _currentSsid[33] = {0};

    uint32_t _listVersion  = 1;
    uint32_t _stateVersion = 1;

    static const Network DUMMY_NET;
};