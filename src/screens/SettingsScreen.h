#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/LayoutService.h"
#include "ui/ButtonBar.h"
#include "services/NightService.h"
#include "services/UiVersionService.h"
#include "services/TimeService.h"
#include "services/PreferencesService.h"

// Типы экрана
#include "screens/settings/SettingsTypes.h"

/*
 * SettingsScreen
 * --------------
 * Экран настроек устройства.
 *
 * Разбиение по файлам:
 *  - SettingsScreen.cpp  — glue / кнопки / lifecycle
 *  - SettingsDraw.cpp    — отрисовка
 *  - SettingsNav.cpp     — навигация
 *  - SettingsEdit.cpp    — изменение значений
 *  - SettingsTypes.h     — enum / POD-типы
 */

class SettingsScreen : public Screen {
public:
    SettingsScreen(
        Adafruit_ST7735& tft,
        ThemeService& themeService,
        LayoutService& layoutService,
        NightService& nightService,
        TimeService& timeService,
        UiVersionService& uiVersion
    );

    // ===== Screen =====
    void begin() override;
    void update() override;

    bool hasStatusBar() const override { return false; }
    bool hasBottomBar() const override { return false; }

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

private:
    // ===== Типы (алиасы для читаемости) =====
    using Level    = SettingsTypes::Level;
    using UiMode   = SettingsTypes::UiMode;
    using HintBtn  = SettingsTypes::HintBtn;
    using MenuItem = SettingsTypes::MenuItem;

private:
    // ===== DRAW =====
    void redrawAll();
    void drawRoot();
    void drawNight();
    void drawTimezone();
    void drawTime();
    void drawButtonHints();

    // ===== NAV / EDIT =====
    void enterSubmenu(Level lvl);
    void exitSubmenu(bool apply);

    void enterEdit();
    void exitEdit(bool apply);

    void navLeft();
    void navRight();
    void editInc();
    void editDec();

    // 🔴 ВАЖНО: helper для SettingsNav.cpp
    int submenuItemsCount() const;

private:
    // ===== Hardware / services =====
    Adafruit_ST7735&  _tft;
    LayoutService&    _layout;
    ButtonBar         _bar;

    NightService&     _night;
    TimeService&      _time;
    UiVersionService& _ui;

    // ===== UI STATE =====
    bool   _dirty = true;
    bool   _exitRequested = false;

    Level  _level = Level::ROOT;
    UiMode _mode  = UiMode::NAV;

    int _selected    = 0;
    int _subSelected = 0;

    // ===== Button feedback =====
    HintBtn _pressedBtn = HintBtn::NONE;
    uint8_t _hintFlash  = 0;

    // ===== Anti-flicker =====
    bool  _needFullClear  = true;
    Level _lastDrawnLevel = Level::ROOT;

    // ===== Time source =====
    TimeService::Mode _tmpTimeMode = TimeService::AUTO;
    TimeService::Mode _bakTimeMode = TimeService::AUTO;

    // ===== ROOT MENU =====
    static constexpr MenuItem MENU[] = {
        { "Wi-Fi",      Level::ROOT     },
        { "Timezone",   Level::TIMEZONE },
        { "Time",       Level::TIME     },
        { "Night mode", Level::NIGHT    },
        { "About",      Level::ROOT     }
    };

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
};