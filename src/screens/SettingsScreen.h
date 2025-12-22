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
    // ===== UI STATE =====
    enum class Level : uint8_t {
        ROOT,
        NIGHT,
        TIMEZONE
    };

    enum class UiMode : uint8_t {
        NAV,
        EDIT
    };

    enum class TzField : uint8_t {
        ZONE,
        DST
    };

    enum class HintBtn : uint8_t {
        NONE,
        LEFT,
        RIGHT,
        OK,
        BACK
    };

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
    // ===== DRAW =====
    void redrawAll();
    void drawRoot();
    void drawNight();
    void drawTimezone();
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

    int _selected = 0;

    // ===== Button highlight =====
    HintBtn  _pressedBtn = HintBtn::NONE;
    uint8_t  _hintFlash  = 0;        // сколько кадров держать подсветку

    // ===== Timezone =====
    TzField _tzField = TzField::ZONE;

    int _tzIndex    = 0;
    int _bakTzIndex = 0;

    // 0=AUTO, 1=ON, 2=OFF
    int _dstAuto    = 0;
    int _bakDstAuto = 0;

    static constexpr MenuItem MENU[] = {
        { "Wi-Fi",      Level::ROOT     },
        { "Timezone",   Level::TIMEZONE },
        { "Night mode", Level::NIGHT    },
        { "About",      Level::ROOT     }
    };

    static constexpr TzItem TZ_LIST[] = {
        { "UTC",       0,        3600 },
        { "Kyiv",      2*3600,   3600 },
        { "Berlin",    1*3600,   3600 },
        { "London",    0,        3600 },
        { "Istanbul",  3*3600,   3600 },
        { "New York", -5*3600,   3600 },
        { "Tokyo",     9*3600,   0    }
    };

    // ===== Night =====
    NightService::Mode _tmpMode;
    NightService::Mode _bakMode;
};