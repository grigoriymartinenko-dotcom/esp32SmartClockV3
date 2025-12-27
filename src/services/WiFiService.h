#pragma once

#include <WiFi.h>
#include <cstdint>
#include <vector>

#include "services/UiVersionService.h"
#include "services/PreferencesService.h"

/*
 * WifiService
 * -----------
 * Контракт "WifiService → UI" (Variant A):
 *
 * 1) WifiService — источник истины:
 *    - собирает данные скана (SSID, RSSI, secured)
 *    - помечает connected / saved
 *    - сортирует список: connected → top, затем по RSSI
 *    - ведёт версии listVersion/stateVersion
 *
 * 2) UI — "тупой художник":
 *    - НЕ трогает WiFi.* (SSID/RSSI/scanNetworks/scanComplete/etc.)
 *    - НЕ сортирует список
 *    - рисует палки сам: rssi(dBm) → bars (Variant A)
 *
 * 3) Версии:
 *    - listVersion++ при любом изменении списка сетей/порядка/connected-флагов
 *    - stateVersion++ при изменении состояния подключения или сканирования
 *
 * 4) Инварианты списка:
 *    - connected == true максимум у одного элемента
 *    - если connected есть в списке — он всегда index 0
 *    - RSSI unknown (INT16_MIN) всегда внизу среди неконнектед
 */

class WifiService {
public:
    // Состояние подключения (высокого уровня, достаточно для UI статусов)
    enum class State {
        OFF,
        CONNECTING,
        ONLINE,
        ERROR
    };

    // Состояние сканирования
    enum class ScanState {
        IDLE,
        SCANNING,
        DONE,
        FAILED
    };

    // RSSI special value: неизвестно/нет данных
    static constexpr int16_t RSSI_UNKNOWN = INT16_MIN;

    // DTO для UI (read-only через networkAt())
    struct Network {
        char    ssid[33];     // null-terminated, "" => invalid/hidden (но мы такие стараемся не выдавать)
        int16_t rssi;         // dBm: -90..-30, RSSI_UNKNOWN если неизвестно
        bool    secured;      // требует пароль (encryption != OPEN)
        bool    connected;    // true у текущей подключенной сети (если она присутствует в списке)
        bool    saved;        // есть сохранённые креды для этого SSID (по prefs)
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

    // true если сейчас WL_CONNECTED
    bool isConnected() const;

    // текущий SSID (если подключены) или nullptr
    const char* currentSsid() const;

    // индекс подключенной сети в списке (после сортировки) или -1
    int connectedIndex() const;

    // ===== CONNECT =====
    void connect(const char* ssid);
    void connect(const char* ssid, const char* pass);
    void disconnect(); // optional, но полезно

    // ===== SCAN =====
    void startScan();
    ScanState scanState() const;

    // ===== LIST (UI CONTRACT) =====
    int networksCount() const;
    const Network& networkAt(int i) const;

    // ===== VERSIONS (UI CONTRACT) =====
    uint32_t listVersion() const;
    uint32_t stateVersion() const;

    // ------------------------------------------------------------------------
    // Legacy helpers (чтобы старый UI не падал). Можно убрать позже.
    // ------------------------------------------------------------------------
    const char* ssidAt(int i) const; // wrapper over networkAt(i).ssid

private:
    void start();
    void stop();

    // bump helpers
    void bumpList();   // listVersion++ + ui.bump(WIFI)
    void bumpState();  // stateVersion++ + ui.bump(WIFI)
    void recomputeConnectedIndex(); // пересчитать _connectedIndex по текущему _networks

    // ===== deps =====
    UiVersionService&    _ui;
    PreferencesService& _prefs;

    // ===== wifi enable/state =====
    State _state = State::OFF;
    bool  _enabled = false;

    // ===== connect timeout =====
    unsigned long _connectStartMs = 0;
    static constexpr unsigned long CONNECT_TIMEOUT_MS = 15000;

    // ===== scan state =====
    ScanState _scanState = ScanState::IDLE;

    // ===== list data (UI reads only) =====
    std::vector<Network> _networks;
    int _connectedIndex = -1;

    // кэш текущего SSID, чтобы UI не лез в WiFi.SSID()
    char _currentSsid[33] = {0};

    // versions
    uint32_t _listVersion  = 1;
    uint32_t _stateVersion = 1;

    // safe dummy network for out-of-range access
    static const Network DUMMY_NET;
};