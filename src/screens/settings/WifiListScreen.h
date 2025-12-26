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
 * UX:
 *  - Заголовок
 *  - ОДНА строка статуса под заголовком (Connected / Reconnecting / Not connected)
 *  - Сепаратор под заголовком и под статусом (локальный, внутри content)
 *  - Список сетей
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
    void onShortLeft() ;
    void onShortRight() ;
    bool hasStatusBar() const override { return false; }
  bool hasBottomBar() const override { return true; }
    //bool hasButtonBar() const override { return true; }

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
    void drawHeader(int baseY);
    void drawConnectionStatus(int baseY);
    void drawScanning(int baseY);
    void drawEmpty(int baseY);
    void drawList(int baseY);
    void drawSeparator(int y);

    int  visibleRows() const;
    bool isConnectedSsid(const char* ssid) const;

    // status tracking (чтобы Connected/Reconnecting обновлялось без bump)
    bool connectionModelDirty();

private:
    Adafruit_ST7735&     _tft;
    LayoutService&      _layout;
    UiVersionService&   _ui;
    WifiService&        _wifi;
    PreferencesService& _prefs;

    ButtonBar _buttons;

    State _state = State::SCANNING;

    int _scroll   = 0;
    int _selected = 0;
    int _netCount = 0;

    uint32_t _lastScreenVer = 0;
    uint32_t _lastWifiVer   = 0;

    // последние значения Wi-Fi соединения (для детекта изменений)
    WifiService::State _lastConnState = WifiService::State::OFF;
    char               _lastConnSsid[40] = {0};
};