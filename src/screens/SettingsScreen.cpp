#include "screens/SettingsScreen.h"
#include <Adafruit_GFX.h>
#include <Arduino.h>

// ============================================================================
// PreferencesService — глобальный объект
// ============================================================================
extern PreferencesService prefs;

// ============================================================================
// static constexpr arrays
// ============================================================================
constexpr SettingsScreen::MenuItem SettingsScreen::MENU[];

// ============================================================================
// ctor
// ============================================================================
SettingsScreen::SettingsScreen(
    Adafruit_ST7735& tft,
    ThemeService& themeService,
    LayoutService& layoutService,
    NightService& nightService,
    TimeService& timeService,
    WifiService& wifiService,
    UiVersionService& uiVersion
)
    : Screen(themeService)
    , _tft(tft)
    , _layout(layoutService)
    , _bar(tft, themeService, layoutService)
    , _night(nightService)
    , _time(timeService)
    , _wifi(wifiService)
    , _ui(uiVersion)
{}

// ============================================================================
// begin
// ============================================================================
void SettingsScreen::begin() {
    _exitRequested = false;

    _level = Level::ROOT;
    _mode  = UiMode::NAV;

    _selected    = 0;
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
// BUTTONS (НЕ РИСУЕМ ЗДЕСЬ)
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
// SUBMENU (логика, БЕЗ draw / nav / edit)
// ============================================================================
void SettingsScreen::enterSubmenu(Level lvl) {
    _level = lvl;
    _mode  = UiMode::NAV;

    _subSelected = 0;
    _needFullClear = true;

    if (lvl == Level::WIFI) {
        _tmpWifiOn = _wifi.isEnabled();
    }

    _dirty = true;
}

void SettingsScreen::exitSubmenu(bool apply) {
    if (_level == Level::WIFI && apply) {
        _wifi.setEnabled(_tmpWifiOn);
    }

    _level = Level::ROOT;
    _mode  = UiMode::NAV;

    _needFullClear = true;
    _dirty = true;
}