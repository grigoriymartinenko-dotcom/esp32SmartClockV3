#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/LayoutService.h"
#include "ui/ButtonBar.h"
#include "services/NightService.h"
#include "services/UiVersionService.h"
#include "services/TimeService.h"
#include "services/PreferencesService.h"

/*
 * SettingsScreen
 * --------------
 * Экран настроек:
 *  - Night mode
 *  - Timezone
 *  - Time source (AUTO / RTC_ONLY / NTP_ONLY / LOCAL_ONLY)
 *
 * Правила:
 *  - без millis()
 *  - без таймеров
 *  - реактивная перерисовка
 *
 * Архитектура (после рефакторинга):
 *  - SettingsScreen.cpp          → логика / nav / edit / state
 *  - settings/SettingsDraw.cpp   → ТОЛЬКО отрисовка (draw*)
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

    bool exitRequested() const;
    void clearExitRequest();

private:
    // ===== UI STATE =====
    enum class Level : uint8_t {
        ROOT,
        NIGHT,
        TIMEZONE,
        TIME
    };

    enum class UiMode : uint8_t {
        NAV,
        EDIT
    };

    enum class HintBtn : uint8_t {
        NONE,
        LEFT,
        RIGHT,
        OK,
        BACK
    };

    struct MenuItem {
        const char* label;
        Level target;
    };

private:
    // ===== DRAW (реализация в settings/SettingsDraw.cpp) =====
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

    // ===== helpers for submenu =====
    int submenuItemsCount() const;

private:
    Adafruit_ST7735&  _tft;
    LayoutService&    _layout;
    ButtonBar         _bar;

    NightService&     _night;
    TimeService&      _time;
    UiVersionService& _ui;

    bool   _dirty = true;
    bool   _exitRequested = false;

    Level  _level = Level::ROOT;
    UiMode _mode  = UiMode::NAV;

    // root selection
    int _selected = 0;

    // submenu selection
    int _subSelected = 0;

    // ===== Button highlight =====
    HintBtn  _pressedBtn = HintBtn::NONE;
    uint8_t  _hintFlash  = 0;

    // ===== Anti-flicker / partial clear =====
    bool  _needFullClear  = true;
    Level _lastDrawnLevel = Level::ROOT;

    // ===== Time source (ЕДИНСТВЕННЫЙ enum) =====
    TimeService::Mode _tmpTimeMode = TimeService::AUTO;
    TimeService::Mode _bakTimeMode = TimeService::AUTO;

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

    // ✅ DST edit-state теперь ВНУТРИ экрана (а не static в .cpp)
    int32_t _tmpDstSec = 0;
    int32_t _bakDstSec = 0;

    static constexpr int32_t TZ_STEP = 900;      // 15 min
    static constexpr int32_t TZ_MIN  = -43200;   // -12:00
    static constexpr int32_t TZ_MAX  =  50400;   // +14:00
};