#pragma once
#include <Adafruit_ST7735.h>
#include <cstring>

#include "core/Screen.h"
#include "services/LayoutService.h"
#include "ui/ButtonBar.h"
#include "services/NightService.h"
#include "services/UiVersionService.h"
#include "services/TimeService.h"
#include "services/PreferencesService.h"
#include "services/WifiService.h"

#include "screens/settings/SettingsTypes.h"

/*
 * SettingsScreen
 * --------------
 * Экран настроек устройства.
 *
 * ВАЖНО:
 *  Этот файл — ЕДИНСТВЕННЫЙ источник истины
 *  для ВСЕХ Settings*.cpp файлов.
 */

class SettingsScreen : public Screen {
public:
    SettingsScreen(
        Adafruit_ST7735& tft,
        ThemeService& themeService,
        LayoutService& layoutService,
        NightService& nightService,
        TimeService& timeService,
        WifiService& wifiService,
        UiVersionService& uiVersion
    );

    // ===== Screen =====
    void begin() override;
    void update() override;

    bool hasStatusBar() const override { return false; }
    bool hasBottomBar() const override { return true; }

    void onThemeChanged() override;

    // ===== Buttons =====
    void onShortLeft();
    void onShortRight();
    void onShortOk();
    void onShortBack();
    void onLongOk();
    void onLongBack();

    // ===== Exit =====
    bool exitRequested() const;
    void clearExitRequest();

protected:
    // ===== DRAW =====
    void redrawAll();
    void drawRoot();
    void drawWifi();
    void drawWifiList();
    void drawWifiPassword();
    void drawTime();
    void drawNight();
    void drawTimezone();
    void drawButtonHints();

private:
    using Level    = SettingsTypes::Level;
    using UiMode   = SettingsTypes::UiMode;
    using HintBtn  = SettingsTypes::HintBtn;
    using MenuItem = SettingsTypes::MenuItem;

    // ===== NAVIGATION =====
    void navLeft();
    void navRight();

    // ===== EDIT MODE (реализовано в SettingsEdit.cpp) =====
    void enterEdit();
    void exitEdit(bool apply);
    void editInc();
    void editDec();

    // ===== SUBMENU =====
    void enterSubmenu(Level lvl);
    void exitSubmenu(bool apply);

    // ===== WIFI HANDLERS (SettingsWifi.cpp) =====
    bool handleWifiShortOk();
    bool handleWifiShortBack();
    bool handleWifiLongOk();
    bool handleWifiLongBack();

private:
    // ===== SERVICES =====
    Adafruit_ST7735&  _tft;
    LayoutService&    _layout;
    ButtonBar         _bar;

    NightService&     _night;
    TimeService&      _time;
    WifiService&      _wifi;
    UiVersionService& _ui;

    // ===== UI STATE =====
    bool   _dirty = true;
    bool   _exitRequested = false;

    Level  _level = Level::ROOT;
    UiMode _mode  = UiMode::NAV;

    // ===== ROOT / SUBMENU CURSORS =====
    int _selected    = 0;
    int _subSelected = 0;

    // ===== WIFI LIST =====
    int _wifiListTop      = 0;
    int _wifiListSelected = 0;

    // ===== WIFI PASSWORD =====
    static constexpr int WIFI_PASS_MAX = 32;
    char _wifiPass[WIFI_PASS_MAX + 1]{};
    int  _wifiPassLen = 0;
    int  _wifiCharIdx = 0;

    static constexpr const char* PASS_CHARS =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "_-@.!";

    // ===== BUTTON FEEDBACK =====
    HintBtn _pressedBtn = HintBtn::NONE;
    uint8_t _hintFlash  = 0;

    // ===== ANTI-FLICKER =====
    bool  _needFullClear  = true;
    Level _lastDrawnLevel = Level::ROOT;

    // ===== ROOT MENU =====
    static constexpr MenuItem MENU[] = {
        { "Wi-Fi",      Level::WIFI     },
        { "Timezone",   Level::TIMEZONE },
        { "Time",       Level::TIME     },
        { "Night mode", Level::NIGHT    },
        { "About",      Level::ROOT     }
    };

    // ===== TIME =====
    TimeService::Mode _tmpTimeMode = TimeService::AUTO;
    TimeService::Mode _bakTimeMode = TimeService::AUTO;

    // ===== NIGHT =====
    NightService::Mode _tmpMode = NightService::Mode::AUTO;
    NightService::Mode _bakMode = NightService::Mode::AUTO;

    int _tmpNightStart = 22 * 60;
    int _tmpNightEnd   = 6 * 60;

    int _bakNightStart = 22 * 60;
    int _bakNightEnd   = 6 * 60;

    static constexpr int NIGHT_STEP_MIN = 15;

    // ===== TIMEZONE =====
    int32_t _tmpTzSec = 0;
    int32_t _bakTzSec = 0;

    int32_t _tmpDstSec = 0;
    int32_t _bakDstSec = 0;

    static constexpr int32_t TZ_STEP = 900;
    static constexpr int32_t TZ_MIN  = -43200;
    static constexpr int32_t TZ_MAX  =  50400;

    // ===== WIFI =====
    bool _tmpWifiOn = true;
    bool _bakWifiOn = true;
};