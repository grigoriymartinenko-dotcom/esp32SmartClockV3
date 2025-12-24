#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/LayoutService.h"
#include "ui/ButtonBar.h"
#include "services/NightService.h"
#include "services/UiVersionService.h"
#include "services/TimeService.h"
#include "services/PreferencesService.h"
#include "services/WifiService.h"

// Типы экрана
#include "screens/settings/SettingsTypes.h"

/*
 * SettingsScreen
 * --------------
 * Экран настроек устройства.
 *
 * Разбиение по файлам:
 *  - SettingsScreen.cpp        — glue / lifecycle / кнопки
 *  - settings/SettingsDraw.cpp — отрисовка
 *  - settings/SettingsNav.cpp  — навигация
 *  - settings/SettingsEdit.cpp — редактирование
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

    void drawWifiList();
private:
    // ===== Типы =====
    using Level    = SettingsTypes::Level;
    using UiMode   = SettingsTypes::UiMode;
    using HintBtn  = SettingsTypes::HintBtn;
    using MenuItem = SettingsTypes::MenuItem;


    // ===== DRAW (SettingsDraw.cpp) =====

protected:
    // ===== DRAW (implemented in settings/*.cpp) =====
protected:
    void redrawAll();
    void drawWifi();
    void drawRoot();
    void drawTime();
    void drawNight();
    void drawTimezone();
    void drawButtonHints();
private:
    // ===== NAV / EDIT =====
    void navLeft();
    void navRight();
    void enterEdit();
    void exitEdit(bool apply);
    void editInc();
    void editDec();
    int  submenuItemsCount() const;

    void enterSubmenu(Level lvl);
    void exitSubmenu(bool apply);

private:
    // ===== Hardware / services =====
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

// ===== Wi-Fi =====
    int _selected    = 0;   // ROOT menu cursor
    int _subSelected = 0;   // submenu cursor
    int _wifiListTop = 0;   // индекс верхней видимой строки списка Wi-Fi

    // ===== Button feedback =====
    HintBtn _pressedBtn = HintBtn::NONE;
    uint8_t _hintFlash  = 0;

    // ===== Anti-flicker =====
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
    int32_t _tmpTzSec = 0;
    int32_t _bakTzSec = 0;

    int32_t _tmpDstSec = 0;
    int32_t _bakDstSec = 0;

    static constexpr int32_t TZ_STEP = 900;
    static constexpr int32_t TZ_MIN  = -43200;
    static constexpr int32_t TZ_MAX  =  50400;

    // ===== Wi-Fi =====
    bool _tmpWifiOn = true;
    bool _bakWifiOn = true;
    // ===== Wi-Fi list =====
// выбранный SSID в списке сканирования
int _wifiListSelected = 0;
};