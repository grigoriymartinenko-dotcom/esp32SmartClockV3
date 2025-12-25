#pragma once

#include <Adafruit_ST7735.h>

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
 * UX:
 *  - Подключённая сеть помечается [Connected]
 *  - Нижнего текста "Connected" НЕТ
 *
 * OK:
 *  - на подключённой сети → Disconnect (Wi-Fi OFF)
 *  - на другой сети        → Connect
 */

class WifiListScreen : public Screen {
public:
    WifiListScreen(
        Adafruit_ST7735& tft,
        ThemeService& theme,
        LayoutService& layout,
        UiVersionService& ui,
        WifiService& wifi,
        PreferencesService& prefs
    );

    // Screen
    void begin() override;
    void update() override;

    bool hasStatusBar() const override { return false; }
    bool hasBottomBar() const override { return false; }
bool hasButtonBar() const override { return true; }

    // Buttons
    void onShortOk();
    void onShortBack();

private:
    enum class State {
        SCANNING,
        READY,
        EMPTY
    };

    // drawing
    void drawHeader();
    void drawScanning();
    void drawEmpty();
    void drawList();
    void drawSeparator(int y);

    int  visibleRows() const;
    bool isConnectedSsid(const char* ssid) const;

private:
    Adafruit_ST7735&    _tft;
    LayoutService&     _layout;
    UiVersionService&  _ui;
    WifiService&       _wifi;
    PreferencesService&_prefs;

    ButtonBar _buttons;

    State _state = State::SCANNING;

    int _scroll   = 0;
    int _selected = 0;
    int _netCount = 0;

    uint32_t _lastScreenVer = 0;
    uint32_t _lastWifiVer   = 0;
};