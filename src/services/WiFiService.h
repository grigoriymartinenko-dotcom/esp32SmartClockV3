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
 *
 * ВАЖНО:
 *  Scan имеет ТРИ состояния:
 *   1) idle      — ещё не запускался
 *   2) scanning  — идёт асинхронный scan
 *   3) finished  — scan завершён (даже если сетей 0)
 *
 * UI НИКОГДА не должен угадывать состояние.
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

    // состояние подключения
    State state() const;

    // ===== SCAN =====
    void startScan();

    bool isScanning() const;        // идёт ли scan прямо сейчас
    bool isScanFinished() const;    // scan завершён (успех или ошибка)

    int  networksCount() const;
    const char* ssidAt(int i) const;

private:
    void start();
    void stop();

    // ===== scan state =====
    bool _scanInProgress = false;   // scan запущен и ещё не завершён
    bool _scanFinished  = false;   // scan завершён (ДАЖЕ если сетей 0)
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