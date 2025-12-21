#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/LayoutService.h"
#include "ui/ButtonBar.h"
#include "services/NightService.h"
#include "services/UiVersionService.h"
#include "services/TimeService.h"
#include "services/PreferencesService.h"

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
    enum class Level : uint8_t {
        ROOT,
        NIGHT,
        TIMEZONE
    };

    enum class UiMode : uint8_t {
        NAV,
        EDIT
    };

    // ===== Night =====
    enum class NightField : uint8_t {
        MODE,
        START,
        END
    };

    enum class TimePart : uint8_t {
        HH,
        MM
    };

    // ===== Timezone =====
    struct TzItem {
        const char* name;
        long gmtOffset;
        int  dstOffset;
    };

    struct MenuItem {
        const char* label;
        Level target;
    };

private:
    // ===== Core =====
    void redrawAll();
    void drawRoot();
    void drawNight();
    void drawTimezone();

    void enterSubmenu(Level lvl);
    void exitSubmenu(bool apply);

    void enterEdit();
    void exitEdit(bool apply);

    void navLeft();
    void navRight();
    void editInc();
    void editDec();

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

    // ===== ROOT =====
    int _selected = 0;

    static constexpr MenuItem MENU[] = {
        { "Wi-Fi",     Level::ROOT     },
        { "Timezone",  Level::TIMEZONE },
        { "Night mode",Level::NIGHT    },
        { "About",     Level::ROOT     }
    };

    // ===== Night state =====
    NightField _nightField = NightField::MODE;
    TimePart   _timePart   = TimePart::HH;

    NightService::Mode _tmpMode;
    int _tmpStartMin = 0;
    int _tmpEndMin   = 0;

    NightService::Mode _bakMode;
    int _bakStartMin;
    int _bakEndMin;

    // ===== Timezone state =====
    int _tzIndex = 0;
    int _bakTzIndex = 0;

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