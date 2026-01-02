#include "screens/SettingsScreen.h"
#include <Arduino.h>
#include <cstring>

extern PreferencesService prefs;

// static constexpr arrays
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
    BrightnessService& brightnessService,
    UiVersionService& uiVersion,
    ButtonBar& buttonBar
)
    : Screen(themeService)
    , _tft(tft)
    , _layout(layoutService)
    , _night(nightService)
    , _time(timeService)
    , _wifi(wifiService)
    , _brightness(brightnessService)
    , _ui(uiVersion)
    , _buttons(buttonBar)
{
}

// ============================================================================
// begin
// ============================================================================
void SettingsScreen::begin() {

    _exitRequested = false;

    _level = Level::ROOT;
    _mode  = UiMode::NAV;

    _selected    = 0;
    _subSelected = 0;

    // ===== Brightness =====
    _tmpBrightness = _brightness.get();
    _bakBrightness = _tmpBrightness;

    _wifiListTop      = 0;
    _wifiListSelected = 0;

    memset(_wifiPass, 0, sizeof(_wifiPass));
    _wifiPassLen = 0;
    _wifiCharIdx = 0;

    // ===== Night =====
    _bakMode = _night.mode();
    _tmpMode = _bakMode;

    _bakNightStart = _night.autoStart();
    _bakNightEnd   = _night.autoEnd();

    _tmpNightStart = _bakNightStart;
    _tmpNightEnd   = _bakNightEnd;

    // ===== Time =====
    _bakTimeMode = _time.mode();
    _tmpTimeMode = _bakTimeMode;

    _lastWifiListVersion  = _wifi.listVersion();
    _lastWifiStateVersion = _wifi.stateVersion();

    updateButtonBarContext();
    _buttons.markDirty();

    _needFullClear  = true;
    _dirty          = true;
    _lastDrawnLevel = _level;

    _ui.bump(UiChannel::SCREEN);
}

// ============================================================================
// update
// ============================================================================
void SettingsScreen::update() {

    if (_wifi.listVersion() != _lastWifiListVersion ||
        _wifi.stateVersion() != _lastWifiStateVersion) {

        _lastWifiListVersion  = _wifi.listVersion();
        _lastWifiStateVersion = _wifi.stateVersion();
        _dirty = true;
    }

    if (_hintFlash > 0)
        _hintFlash--;

    if (_dirty) {
        _dirty = false;
        redrawAll();
    }
}

// ============================================================================
// BUTTON HANDLERS
// ============================================================================
void SettingsScreen::onShortLeft() {
    _pressedBtn = HintBtn::LEFT;
    _hintFlash  = 3;

    if (_mode == UiMode::EDIT) { editDec(); return; }
    navLeft();
}

void SettingsScreen::onShortRight() {
    _pressedBtn = HintBtn::RIGHT;
    _hintFlash  = 3;

    if (_mode == UiMode::EDIT) { editInc(); return; }
    navRight();
}

void SettingsScreen::onShortOk() {
    _pressedBtn = HintBtn::OK;
    _hintFlash  = 3;

    if (_level == Level::ROOT) {
        enterSubmenu(MENU[_selected].target);
        return;
    }

    if (_mode == UiMode::NAV) {
        enterEdit();
        return;
    }

    _dirty = true;
}

void SettingsScreen::onShortBack() {
    _pressedBtn = HintBtn::BACK;
    _hintFlash  = 3;

    if (handleWifiShortBack())
        return;

    if (_level != Level::ROOT) {
        enterSubmenu(Level::ROOT);
        return;
    }

    _exitRequested = true;
}

void SettingsScreen::onLongOk() {

    // ===== APPLY BRIGHTNESS =====
    if (_mode == UiMode::EDIT && _level == Level::BRIGHTNESS) {
        _bakBrightness = _tmpBrightness;
        exitEdit(true);
        return;
    }

    if (_mode == UiMode::EDIT) {

        // ===== APPLY NIGHT =====
        if (_level == Level::NIGHT) {

            _night.setMode(_tmpMode);

            if (_tmpMode == NightService::Mode::AUTO) {
                _night.setAutoRange(_tmpNightStart, _tmpNightEnd);
            }

            prefs.setNightMode((NightModePref)_tmpMode);
            prefs.setNightAutoRange(_tmpNightStart, _tmpNightEnd);
            prefs.save();

            _ui.bump(UiChannel::THEME);
        }

        // ===== APPLY TIME =====
        if (_level == Level::TIME) {
            _time.setMode(_tmpTimeMode);
            _ui.bump(UiChannel::TIME);
        }

        exitEdit(true);
        return;
    }
}

void SettingsScreen::onLongBack() {

    // ===== CANCEL BRIGHTNESS =====
    if (_mode == UiMode::EDIT && _level == Level::BRIGHTNESS) {
        _brightness.set(_bakBrightness);
        exitEdit(false);
        return;
    }

    // ===== CANCEL TIME =====
    if (_mode == UiMode::EDIT && _level == Level::TIME) {
        _tmpTimeMode = _bakTimeMode;
        _time.setMode(_bakTimeMode);
        exitEdit(false);
        _buttons.markDirty();
        return;
    }

    if (_mode == UiMode::EDIT) {
        exitEdit(false);
        _buttons.markDirty();
        return;
    }

    if (handleWifiLongBack())
        return;

    enterSubmenu(Level::ROOT);
}

// ============================================================================
// EXIT
// ============================================================================
bool SettingsScreen::exitRequested() const {
    return _exitRequested;
}

void SettingsScreen::clearExitRequest() {
    _exitRequested = false;
}

// ============================================================================
// THEME
// ============================================================================
void SettingsScreen::onThemeChanged() {
    _needFullClear = true;
    _dirty = true;
    _buttons.markDirty();
}

// ============================================================================
// SUBMENU
// ============================================================================
void SettingsScreen::enterSubmenu(Level lvl) {

    if (_level == lvl)
        return;

    _level = lvl;
    _mode  = UiMode::NAV;
    _subSelected = 0;

    if (_level == Level::BRIGHTNESS) {
        _tmpBrightness = _brightness.get();
        _bakBrightness = _tmpBrightness;
    }

    if (_level == Level::NIGHT) {
        _bakMode = _night.mode();
        _tmpMode = _bakMode;

        _bakNightStart = _night.autoStart();
        _bakNightEnd   = _night.autoEnd();

        _tmpNightStart = _bakNightStart;
        _tmpNightEnd   = _bakNightEnd;
    }

    if (_level == Level::TIME) {
        _bakTimeMode = _time.mode();
        _tmpTimeMode = _bakTimeMode;
    }

    _needFullClear = true;
    _dirty         = true;

    _lastWifiListTop      = -1;
    _lastWifiListSelected = -1;
    _lastWifiNetCount     = -1;

    updateButtonBarContext();
    _buttons.markDirty();
}