#pragma once
#include <WiFi.h>

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

    WifiService(
        UiVersionService& ui,
        PreferencesService& prefs
    );

    void begin();
    void update();

    void setEnabled(bool on);
    bool isEnabled() const;

    State state() const;

private:
    void start();
    void stop();

private:
    UiVersionService& _ui;
    PreferencesService& _prefs;

    State _state = State::OFF;
    bool  _enabled = false;

    unsigned long _connectStartMs = 0;
    static constexpr unsigned long CONNECT_TIMEOUT_MS = 15000;
};