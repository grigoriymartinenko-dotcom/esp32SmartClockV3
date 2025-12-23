#include "screens/SettingsScreen.h"
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// PreferencesService — глобальный объект
// ============================================================================
extern PreferencesService prefs;

// ============================================================================
// TIMEZONE: DST edit temp (НЕ ТРЕБУЕТ правок SettingsScreen.h)
// ============================================================================
static int32_t s_tmpDstSec = 0;
static int32_t s_bakDstSec = 0;

// ============================================================================
// static constexpr arrays
// ============================================================================
constexpr SettingsScreen::MenuItem SettingsScreen::MENU[];

// ============================================================================
// small helpers
// ============================================================================
static void formatOffsetHM(int32_t sec, char* out, size_t outSz) {
    int32_t s = sec;
    char sign = '+';
    if (s < 0) { sign = '-'; s = -s; }

    int hh = (int)(s / 3600);
    int mm = (int)((s % 3600) / 60);

    snprintf(out, outSz, "%c%02d:%02d", sign, hh, mm);
}

static int32_t clampI32(int32_t v, int32_t lo, int32_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int clampMin(int v) {
    // wrap 0..1439
    while (v < 0)     v += 1440;
    while (v >= 1440) v -= 1440;
    return v;
}

// ============================================================================
// ctor
// ============================================================================
SettingsScreen::SettingsScreen(
    Adafruit_ST7735& tft,
    ThemeService& themeService,
    LayoutService& layoutService,
    NightService& nightService,
    TimeService& timeService,
    UiVersionService& uiVersion
)
    : Screen(themeService)
    , _tft(tft)
    , _layout(layoutService)
    , _bar(tft, themeService, layoutService)
    , _night(nightService)
    , _time(timeService)
    , _ui(uiVersion)
{}

// ============================================================================
// begin
// ============================================================================
void SettingsScreen::begin() {
    _exitRequested = false;
    _level = Level::ROOT;
    _mode  = UiMode::NAV;

    _selected = 0;
    _subSelected = 0;

    _dirty = true;

    _needFullClear  = true;
    _lastDrawnLevel = _level;

    _bar.setVisible(false);
}

// ============================================================================
// update
// ============================================================================
void SettingsScreen::update() {
    if (_hintFlash > 0) {
        _hintFlash--;
        drawButtonHints();
    }

    if (_dirty) {
        _dirty = false;
        redrawAll();
    }
}

void SettingsScreen::onThemeChanged() {
    _needFullClear = true;
    _dirty = true;
}

// ============================================================================
// BUTTONS (НЕ РИСУЕМ здесь!)
// ============================================================================
void SettingsScreen::onShortLeft() {
    _pressedBtn = HintBtn::LEFT;
    _hintFlash  = 3;

    if (_mode == UiMode::NAV) navLeft();
    else                      editDec();
}

void SettingsScreen::onShortRight() {
    _pressedBtn = HintBtn::RIGHT;
    _hintFlash  = 3;

    if (_mode == UiMode::NAV) navRight();
    else                      editInc();
}

void SettingsScreen::onShortOk() {
    _pressedBtn = HintBtn::OK;
    _hintFlash  = 3;

    if (_mode == UiMode::NAV && _level == Level::ROOT) {
        enterSubmenu(MENU[_selected].target);
        return;
    }

    if (_mode == UiMode::NAV) {
        enterEdit();
        return;
    }
}

void SettingsScreen::onLongOk() {
    if (_mode == UiMode::EDIT) exitEdit(true);
}

void SettingsScreen::onShortBack() {
    _pressedBtn = HintBtn::BACK;
    _hintFlash  = 3;

    if (_mode == UiMode::EDIT) {
        exitEdit(false);
        return;
    }

    if (_level == Level::ROOT) _exitRequested = true;
    else                       exitSubmenu(true);
}

void SettingsScreen::onLongBack() {}

// ============================================================================
// EXIT FLAGS
// ============================================================================
bool SettingsScreen::exitRequested() const {
    return _exitRequested;
}

void SettingsScreen::clearExitRequest() {
    _exitRequested = false;
}

// ============================================================================
// SUBMENU
// ============================================================================
void SettingsScreen::enterSubmenu(Level lvl) {
    _level = lvl;
    _mode  = UiMode::NAV;

    _subSelected = 0;     // всегда первый пункт

    _needFullClear = true;

    if (lvl == Level::TIME) {
        _tmpTimeMode = (TimeService::Mode)prefs.timeSource();
        _bakTimeMode = _tmpTimeMode;
    }

    if (lvl == Level::NIGHT) {
        _tmpMode = _night.mode();
        _bakMode = _tmpMode;

        _tmpNightStart = (int)prefs.nightStart();
        _tmpNightEnd   = (int)prefs.nightEnd();

        _bakNightStart = _tmpNightStart;
        _bakNightEnd   = _tmpNightEnd;
    }

    if (lvl == Level::TIMEZONE) {
        _tmpTzSec = prefs.tzGmtOffset();
        _bakTzSec = _tmpTzSec;

        s_tmpDstSec = prefs.tzDstOffset();
        s_bakDstSec = s_tmpDstSec;
    }

    _dirty = true;
}

void SettingsScreen::exitSubmenu(bool apply) {
    if (_level == Level::TIME && apply) {
        prefs.setTimeSource((TimeSourcePref)_tmpTimeMode);
        prefs.save();
        _time.setMode(_tmpTimeMode);
    }

    if (_level == Level::NIGHT && apply) {
        _night.setMode(_tmpMode);
        _night.setAutoRange(_tmpNightStart, _tmpNightEnd);

        prefs.setNightMode(
            (_tmpMode == NightService::Mode::AUTO) ? NightModePref::AUTO :
            (_tmpMode == NightService::Mode::ON)   ? NightModePref::ON
                                                   : NightModePref::OFF
        );

        prefs.setNightRange((uint16_t)_tmpNightStart, (uint16_t)_tmpNightEnd);
        prefs.save();
    }

    _level = Level::ROOT;
    _mode  = UiMode::NAV;

    _needFullClear = true;
    _dirty = true;
}
