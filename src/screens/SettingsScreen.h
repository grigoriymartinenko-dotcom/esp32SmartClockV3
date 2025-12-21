#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/LayoutService.h"
#include "ui/ButtonBar.h"
#include "services/NightService.h"
#include "services/UiVersionService.h"

/*
 * SettingsScreen
 * --------------
 * Root Settings + Night Mode submenu
 * 4 hardware buttons UX
 */

class SettingsScreen : public Screen {
public:
    SettingsScreen(
        Adafruit_ST7735& tft,
        ThemeService& themeService,
        LayoutService& layoutService,
        NightService& nightService,
        UiVersionService& uiVersion
    );

    // ===== Screen =====
    void begin() override;
    void update() override;

    bool hasStatusBar() const override { return false; }
    bool hasBottomBar() const override { return false; }

    void onThemeChanged() override;

    // ===== Buttons (used by AppController) =====
    void onLeft();
    void onRight();
    void onOk();
    void onBack();

    void onOkLong();
    void onBackLong();

    // ===== Exit handling =====
    bool exitRequested() const;
    void clearExitRequest();

private:
    // ===== Internal levels =====
    enum class Level : uint8_t {
        ROOT,
        NIGHT_MODE
    };

    enum class NightField : uint8_t {
        MODE = 0,
        START,
        END
    };

    enum class TimePart : uint8_t {
        HH,
        MM
    };

private:
    // ===== Draw =====
    void redrawAll();
    void drawTitle();
    void drawList();
    void drawNightMenu();

    // ===== Night submenu logic =====
    void enterNightMenu();
    void exitNightMenu(bool apply);

    void nightLeft();
    void nightRight();
    void nightUp();
    void nightDown();
    void nightEnter();   // LONG OK
    void nightExit();    // LONG BACK

private:
    Adafruit_ST7735&  _tft;
    LayoutService&    _layout;
    ButtonBar         _bar;
    NightService&     _night;
    UiVersionService& _ui;

    bool _dirty = true;
    bool _exitRequested = false;

    // ===== Root menu =====
    static constexpr int ITEM_COUNT = 6;
    int _selected = 0;

    // ===== Submenu state =====
    Level _level = Level::ROOT;

    NightField _nightField = NightField::MODE;
    TimePart   _timePart   = TimePart::HH;
    bool       _editing    = false;

    // temp editable values
    NightService::Mode _tmpMode;
    int _tmpStartMin = 0;
    int _tmpEndMin   = 0;
};