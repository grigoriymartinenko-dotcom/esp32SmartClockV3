#pragma once
#include <Adafruit_ST7735.h>
#include <cstring>

#include "core/Screen.h"
#include "services/LayoutService.h"
#include "services/NightService.h"
#include "services/UiVersionService.h"
#include "services/TimeService.h"
#include "services/PreferencesService.h"
#include "services/WifiService.h"
#include "ui/ButtonBar.h"

#include "screens/settings/SettingsTypes.h"

class SettingsScreen : public Screen {
public:
    SettingsScreen(
        Adafruit_ST7735& tft,
        ThemeService& themeService,
        LayoutService& layoutService,
        NightService& nightService,
        TimeService& timeService,
        WifiService& wifiService,
        UiVersionService& uiVersion,
        ButtonBar& buttonBar
    );

    void begin() override;
    void update() override;

    bool hasStatusBar() const override { return true; }
    bool hasButtonBar() const override { return true; }

    void onThemeChanged() override;

    void onShortLeft();
    void onShortRight();
    void onShortOk();
    void onShortBack();
    void onLongOk();
    void onLongBack();

    bool exitRequested() const;
    void clearExitRequest();

protected:
    void redrawAll();

    void drawRoot();
    void drawWifi();
    void drawWifiList();
    void drawWifiPassword();
    void drawTime();
    void drawNight();
    void drawTimezone();

private:
    using Level    = SettingsTypes::Level;
    using UiMode   = SettingsTypes::UiMode;
    using HintBtn  = SettingsTypes::HintBtn;
    using MenuItem = SettingsTypes::MenuItem;

    void updateButtonBarContext();

    void navLeft();
    void navRight();

    void enterEdit();
    void exitEdit(bool apply);
    void editInc();
    void editDec();

    void enterSubmenu(Level lvl);
    void exitSubmenu(bool apply);

    bool handleWifiShortOk();
    bool handleWifiShortBack();
    bool handleWifiLongOk();
    bool handleWifiLongBack();

    // 👇 ДОБАВЛЕНО: применение Night mode
    void applyNightSettings();

private:
    Adafruit_ST7735&  _tft;
    LayoutService&    _layout;

    NightService&     _night;
    TimeService&      _time;
    WifiService&      _wifi;
    UiVersionService& _ui;
    ButtonBar&        _buttons;

    bool   _dirty = true;
    bool   _exitRequested = false;

    Level  _level = Level::ROOT;
    UiMode _mode  = UiMode::NAV;

    int _selected    = 0;
    int _subSelected = 0;

    // ===== Wi-Fi list =====
    int _wifiListTop      = 0;
    int _wifiListSelected = 0;

    static constexpr int WIFI_PASS_MAX = 32;
    char _wifiPass[WIFI_PASS_MAX + 1]{};
    int  _wifiPassLen = 0;
    int  _wifiCharIdx = 0;

    static constexpr const char* PASS_CHARS =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "_-@.!";

    // ===== Menu =====
    static constexpr MenuItem MENU[] = {
        { "Wi-Fi",      Level::WIFI     },
        { "Timezone",   Level::TIMEZONE },
        { "Time",       Level::TIME     },
        { "Night mode", Level::NIGHT    },
        { "About",      Level::ROOT     }
    };

    // ===== Time =====
    TimeService::Mode _tmpTimeMode = TimeService::AUTO;
    TimeService::Mode _bakTimeMode = TimeService::AUTO;

    // ===== Night =====
    NightService::Mode _tmpMode = NightService::Mode::AUTO;
    NightService::Mode _bakMode = NightService::Mode::AUTO;

    int _tmpNightStart = 22 * 60;
    int _tmpNightEnd   = 6 * 60;

    int _bakNightStart = 22 * 60;
    int _bakNightEnd   = 6 * 60;

    static constexpr int NIGHT_STEP_MIN = 15;

    // ===== Timezone =====
    int32_t _tmpTzSec  = 0;
    int32_t _bakTzSec  = 0;

    int32_t _tmpDstSec = 0;
    int32_t _bakDstSec = 0;

    static constexpr int32_t TZ_STEP = 900;
    static constexpr int32_t TZ_MIN  = -43200;
    static constexpr int32_t TZ_MAX  =  50400;

    // ===== Wi-Fi enable =====
    bool _tmpWifiOn = true;
    bool _bakWifiOn = true;

    // ===== Versions =====
    uint32_t _lastWifiListVersion  = 0;
    uint32_t _lastWifiStateVersion = 0;

    // ===== UI cache =====
    int _lastWifiListTop      = -1;
    int _lastWifiListSelected = -1;
    int _lastWifiNetCount     = -1;

    HintBtn _pressedBtn = HintBtn::NONE;
    uint8_t _hintFlash  = 0;

    bool  _needFullClear  = true;
    Level _lastDrawnLevel = Level::ROOT;
};