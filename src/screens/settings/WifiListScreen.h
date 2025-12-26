#pragma once

#include <Adafruit_ST7735.h>
#include <cstring>

#include "core/Screen.h"
#include "services/LayoutService.h"
#include "services/UiVersionService.h"
#include "services/WifiService.h"
#include "services/PreferencesService.h"
#include "ui/ButtonBar.h"

/*
 * WifiListScreen
 * --------------
 * Экран списка Wi-Fi сетей.
 *
 * ButtonBar контекст:
 *
 * SCANNING:
 *      BACK
 *
 * READY (SSID):
 *   <   SELECT   >   BACK
 *
 * READY (RESCAN):
 *   <   RESCAN   >   BACK
 *
 * CONNECTED / RECONNECTING:
 *      BACK
 */

class WifiListScreen : public Screen {
public:
    WifiListScreen(
        Adafruit_ST7735& tft,
        ThemeService& theme,
        LayoutService& layout,
        UiVersionService& ui,
        WifiService& wifi,
        PreferencesService& prefs,
        ButtonBar& buttonBar
    );

    // ===== Screen =====
    void begin() override;
    void update() override;

    bool hasStatusBar() const override { return false; }
    bool hasButtonBar() const override { return true; }

    // ===== Buttons =====
    void onShortLeft();
    void onShortRight();
    void onShortOk();
    void onShortBack();

private:
    enum class State {
        SCANNING,
        READY,
        CONNECTED,
        RECONNECTING
    };

    // ===== Drawing =====
    void redrawAll();
    void drawHeader(int baseY);
    void drawConnectionStatus(int baseY);
    void drawScanning(int baseY);
    void drawList(int baseY);
    void drawSeparator(int y);

    int  visibleRows() const;
    bool isConnectedSsid(const char* ssid) const;

    // ===== ButtonBar =====
    void updateButtonBarContext();

    // ===== Helpers =====
    bool connectionModelDirty();

private:
    Adafruit_ST7735&     _tft;
    LayoutService&      _layout;
    UiVersionService&   _ui;
    WifiService&        _wifi;
    PreferencesService& _prefs;
    ButtonBar&          _buttons;

    State _state = State::SCANNING;

    int _scroll   = 0;
    int _selected = 0;
    int _netCount = 0;

    uint32_t _lastScreenVer = 0;
    uint32_t _lastWifiVer   = 0;

    WifiService::State _lastConnState = WifiService::State::OFF;
    char               _lastConnSsid[40] = {0};
};