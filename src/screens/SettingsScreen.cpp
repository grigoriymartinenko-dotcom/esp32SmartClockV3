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
    UiVersionService& uiVersion,
    ButtonBar& buttonBar
)
    : Screen(themeService)
    , _tft(tft)
    , _layout(layoutService)
    , _night(nightService)
    , _time(timeService)
    , _wifi(wifiService)
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

    _wifiListTop      = 0;
    _wifiListSelected = 0;

    memset(_wifiPass, 0, sizeof(_wifiPass));
    _wifiPassLen = 0;
    _wifiCharIdx = 0;

    // 🔥 ВАЖНО: зафиксировать стартовые версии
    _lastWifiListVersion  = _wifi.listVersion();
    _lastWifiStateVersion = _wifi.stateVersion();

    updateButtonBarContext();

    _dirty = true;
    _needFullClear  = true;
    _lastDrawnLevel = _level;

    _ui.bump(UiChannel::SCREEN);
}

// ============================================================================
// update
// ============================================================================
void SettingsScreen::update() {

    // ------------------------------------------------------------------------
    // Wi-Fi CONTRACT:
    // перерисовываемся ТОЛЬКО если реально что-то изменилось
    // ------------------------------------------------------------------------
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
        redrawAll();   // реализовано в SettingsDraw.cpp
    }
}

// ============================================================================
// BUTTONS
// ============================================================================
void SettingsScreen::onShortLeft() {
    _pressedBtn = HintBtn::LEFT;
    _hintFlash  = 3;

    if (_level == Level::WIFI_PASSWORD) {
        const size_t n = strlen(PASS_CHARS);
        _wifiCharIdx = (_wifiCharIdx == 0)
            ? (int)(n - 1)
            : (_wifiCharIdx - 1);
        _dirty = true;
        return;
    }

    navLeft();
}

void SettingsScreen::onShortRight() {
    _pressedBtn = HintBtn::RIGHT;
    _hintFlash  = 3;

    if (_level == Level::WIFI_PASSWORD) {
        const size_t n = strlen(PASS_CHARS);
        _wifiCharIdx = (int)((_wifiCharIdx + 1) % (int)n);
        _dirty = true;
        return;
    }

    navRight();
}

void SettingsScreen::onShortOk() {
    _pressedBtn = HintBtn::OK;
    _hintFlash  = 3;

    if (_level == Level::ROOT) {
        enterSubmenu(MENU[_selected].target);
        return;
    }

    if (handleWifiShortOk())
        return;

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
    if (handleWifiLongOk())
        return;
}

void SettingsScreen::onLongBack() {
    if (handleWifiLongBack())
        return;
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
}

// ============================================================================
// SUBMENU
// ============================================================================
void SettingsScreen::enterSubmenu(Level lvl) {
    _level = lvl;
    _mode  = UiMode::NAV;
    _subSelected = 0;

    _needFullClear = true;
    _dirty = true;
}

void SettingsScreen::exitSubmenu(bool /*apply*/) {
    enterSubmenu(Level::ROOT);
}

// ============================================================================
// BUTTON BAR
// ============================================================================
void SettingsScreen::updateButtonBarContext() {

    if (_mode == UiMode::NAV) {
        _buttons.setLabels("<", "OK", ">", "BACK");
        _buttons.setActions(true, true, true, true);
        _buttons.setHighlight(false, false, false, false);
    } else {
        // EDIT
        _buttons.setLabels("-", "OK+", "+", "BACK+");
        _buttons.setActions(true, true, true, true);
        _buttons.setHighlight(false, true, false, false);
    }

    _buttons.markDirty();
}