#pragma once

#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/LayoutService.h"
#include "services/UiVersionService.h"
#include "services/WifiService.h"
#include "ui/ButtonBar.h"

/*
 * WifiListScreen
 * --------------
 * Экран списка Wi-Fi сетей.
 *
 * Показывает:
 *  - Scanning…
 *  - Empty list
 *  - List SSID
 *
 * ВАЖНО:
 *  - Экран НЕ обрабатывает кнопки
 *  - Навигация будет подключена позже
 *  - Перерисовка только по версиям
 */

class WifiListScreen : public Screen {
public:
    WifiListScreen(
        Adafruit_ST7735& tft,
        ThemeService& theme,
        LayoutService& layout,
        UiVersionService& ui,
        WifiService& wifi
    );

    // Screen
    void begin() override;
    void update() override;

    bool hasStatusBar() const override { return false; }
    bool hasBottomBar() const override { return true; }

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

    int visibleRows() const;

private:
    Adafruit_ST7735&   _tft;
    LayoutService&    _layout;
    UiVersionService& _ui;
    WifiService&      _wifi;

    ButtonBar _buttons;

    State _state = State::SCANNING;

    int _scroll   = 0;
    int _netCount = 0;

    uint32_t _lastScreenVer = 0;
    uint32_t _lastWifiVer   = 0;
};