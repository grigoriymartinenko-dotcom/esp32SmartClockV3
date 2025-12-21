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
 * Root Settings + Night Mode + Timezone submenu
 * 4 hardware buttons UX
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
    void onLeft();
    void onRight();
    void onOk();
    void onBack();

    void onOkLong();
    void onBackLong();

    // ===== Exit =====
    bool exitRequested() const;
    void clearExitRequest();

private:
    // ===== Levels =====
    enum class Level : uint8_t {
        ROOT,
        NIGHT_MODE,
        TIMEZONE
    };

    // ===== Night submenu =====
    enum class NightField : uint8_t {
        MODE = 0,
        START,
        END
    };

    enum class TimePart : uint8_t {
        HH,
        MM
    };

    // ===== Timezone submenu =====
    struct TzItem {
        const char* name;
        long gmtOffset;
        int  dstOffset;
    };

private:
    // ===== Draw =====
    void redrawAll();
    void drawTitle();
    void drawList();
    void drawNightMenu();
    void drawTimezoneMenu();

    // ===== Night logic =====
    void enterNightMenu();
    void exitNightMenu(bool apply);
    void nightLeft();
    void nightRight();
    void nightUp();
    void nightDown();
    void nightEnter();
    void nightExit();

    // ===== Timezone logic =====
    void enterTimezoneMenu();
    void exitTimezoneMenu(bool apply);
    void tzLeft();
    void tzRight();
    void tzUp();
    void tzDown();
    void tzEnter();
    void tzExit();

private:
    Adafruit_ST7735&  _tft;
    LayoutService&    _layout;
    ButtonBar         _bar;
    NightService&     _night;
    TimeService&      _time;
    UiVersionService& _ui;

    bool _dirty = true;
    bool _exitRequested = false;

    // ===== Root =====
    static constexpr int ITEM_COUNT = 6;
    int _selected = 0;

    // ===== State =====
    Level _level = Level::ROOT;

    // ===== Night state =====
    NightField _nightField = NightField::MODE;
    TimePart   _timePart   = TimePart::HH;
    bool       _editing    = false;

    NightService::Mode _tmpMode;
    int _tmpStartMin = 0;
    int _tmpEndMin   = 0;

    // ===== Timezone state =====
    int  _tzIndex = 0;
    bool _tzEditing = false;

    static constexpr TzItem TZ_LIST[] = {
        { "UTC",       0,        0 },
        { "Kyiv",      2*3600,   0 },
        { "Berlin",    1*3600,   0 },
        { "London",    0,        0 },
        { "Istanbul",  3*3600,   0 },
        { "New York", -5*3600,   0 },
        { "Tokyo",     9*3600,   0 }
    };
};