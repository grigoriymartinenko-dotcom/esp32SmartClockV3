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
// submenu items count
// ============================================================================
int SettingsScreen::submenuItemsCount() const {
    switch (_level) {
        case Level::TIMEZONE: return 2; // UTC, DST
        case Level::TIME:     return 1; // Source
        case Level::NIGHT:    return 3; // Mode, Start, End
        case Level::ROOT:     return (int)(sizeof(MENU) / sizeof(MENU[0]));
    }
    return 1;
}

// ============================================================================
// NAVIGATION
// ============================================================================
void SettingsScreen::navLeft() {
    if (_level == Level::ROOT) {
        int n = sizeof(MENU) / sizeof(MENU[0]);
        _selected = (_selected + n - 1) % n;
        _dirty = true;
        return;
    }

    int n = submenuItemsCount();
    if (n <= 1) return;

    _subSelected = (_subSelected + n - 1) % n;
    _dirty = true;
}

void SettingsScreen::navRight() {
    if (_level == Level::ROOT) {
        int n = sizeof(MENU) / sizeof(MENU[0]);
        _selected = (_selected + 1) % n;
        _dirty = true;
        return;
    }

    int n = submenuItemsCount();
    if (n <= 1) return;

    _subSelected = (_subSelected + 1) % n;
    _dirty = true;
}

// ============================================================================
// EDIT CORE
// ============================================================================
void SettingsScreen::enterEdit() {
    _mode = UiMode::EDIT;

    if (_level == Level::TIME) {
        _bakTimeMode = _tmpTimeMode;
        _dirty = true;
        return;
    }

    if (_level == Level::TIMEZONE) {
        // ✅ Редактируем И UTC И DST (как ты просил)
        _bakTzSec = prefs.tzGmtOffset();
        _tmpTzSec = _bakTzSec;

        s_bakDstSec = prefs.tzDstOffset();
        s_tmpDstSec = s_bakDstSec;

        // НИКАКИХ запретов по _subSelected — обе строки редактируемые
        _dirty = true;
        return;
    }

    if (_level == Level::NIGHT) {
        _bakMode = _tmpMode;
        _bakNightStart = _tmpNightStart;
        _bakNightEnd   = _tmpNightEnd;

        _dirty = true;
        return;
    }

    _dirty = true;
}

void SettingsScreen::exitEdit(bool apply) {
    if (!apply) {
        if (_level == Level::TIME)     _tmpTimeMode   = _bakTimeMode;
        if (_level == Level::TIMEZONE) {
            _tmpTzSec    = _bakTzSec;
            s_tmpDstSec  = s_bakDstSec;
        }
        if (_level == Level::NIGHT) {
            _tmpMode       = _bakMode;
            _tmpNightStart = _bakNightStart;
            _tmpNightEnd   = _bakNightEnd;
        }

        _mode = UiMode::NAV;
        _dirty = true;
        return;
    }

    // APPLY для Timezone — прямо тут
    if (_level == Level::TIMEZONE) {
        prefs.setTimezone(_tmpTzSec, s_tmpDstSec);
        prefs.save();
        _time.setTimezone((long)_tmpTzSec, (int)s_tmpDstSec);
    }

    // Night/Time сохраняем при выходе из сабменю (exitSubmenu)
    _mode = UiMode::NAV;
    _dirty = true;
}

// ============================================================================
// EDIT ACTIONS
// ============================================================================
void SettingsScreen::editInc() {
    if (_level == Level::TIME) {
        int v = (int)_tmpTimeMode;
        _tmpTimeMode = (TimeService::Mode)((v + 1) % 4);
        _dirty = true;
        return;
    }

    if (_level == Level::TIMEZONE) {
        // ✅ LEFT/RIGHT меняют выбранную строку: UTC или DST
        if (_subSelected == 0) {
            _tmpTzSec = clampI32(_tmpTzSec + TZ_STEP, TZ_MIN, TZ_MAX);
        } else if (_subSelected == 1) {
            s_tmpDstSec = clampI32(s_tmpDstSec + TZ_STEP, TZ_MIN, TZ_MAX);
        }
        _dirty = true;
        return;
    }

    if (_level == Level::NIGHT) {
        if (_subSelected == 0) {
            int v = (int)_tmpMode;
            _tmpMode = (NightService::Mode)((v + 1) % 3);
            _dirty = true;
            return;
        }

        if (_subSelected == 1) {
            _tmpNightStart = clampMin(_tmpNightStart + NIGHT_STEP_MIN);
            _dirty = true;
            return;
        }

        if (_subSelected == 2) {
            _tmpNightEnd = clampMin(_tmpNightEnd + NIGHT_STEP_MIN);
            _dirty = true;
            return;
        }
    }
}

void SettingsScreen::editDec() {
    if (_level == Level::TIME) {
        int v = (int)_tmpTimeMode;
        _tmpTimeMode = (TimeService::Mode)((v + 3) % 4);
        _dirty = true;
        return;
    }

    if (_level == Level::TIMEZONE) {
        // ✅ LEFT/RIGHT меняют выбранную строку: UTC или DST
        if (_subSelected == 0) {
            _tmpTzSec = clampI32(_tmpTzSec - TZ_STEP, TZ_MIN, TZ_MAX);
        } else if (_subSelected == 1) {
            s_tmpDstSec = clampI32(s_tmpDstSec - TZ_STEP, TZ_MIN, TZ_MAX);
        }
        _dirty = true;
        return;
    }

    if (_level == Level::NIGHT) {
        if (_subSelected == 0) {
            int v = (int)_tmpMode;
            _tmpMode = (NightService::Mode)((v + 2) % 3);
            _dirty = true;
            return;
        }

        if (_subSelected == 1) {
            _tmpNightStart = clampMin(_tmpNightStart - NIGHT_STEP_MIN);
            _dirty = true;
            return;
        }

        if (_subSelected == 2) {
            _tmpNightEnd = clampMin(_tmpNightEnd - NIGHT_STEP_MIN);
            _dirty = true;
            return;
        }
    }
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

// ============================================================================
// DRAW
// ============================================================================


// ============================================================================
// ROOT
// ============================================================================
