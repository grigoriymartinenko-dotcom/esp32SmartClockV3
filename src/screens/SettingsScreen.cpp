#include "screens/SettingsScreen.h"
#include <Adafruit_GFX.h>
#include <Arduino.h>

/*
 * SettingsScreen.cpp
 * ------------------
 * ЛОГИКА экрана Settings.
 *
 * ПРАВИЛА:
 *  - НИКАКОЙ отрисовки (это в SettingsDraw.cpp)
 *  - НИКАКИХ delay / таймеров
 *  - ТОЛЬКО реакция на кнопки и смена UI-состояния
 */

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

    /*
     * WiFiService делает _ui.bump(UiChannel::WIFI), когда:
     *  - меняется статус Wi-Fi
     *  - начинается scan
     *  - scan завершается
     *
     * Мы обязаны перерисовать экран.
     */
    if (_ui.changed(UiChannel::WIFI)) {
        _dirty = true;
    }

    if (_hintFlash > 0) {
        _hintFlash--;
        drawButtonHints();
    }

    if (_dirty) {
        _dirty = false;
        redrawAll();
    }
}

// ============================================================================
// BUTTONS
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

    // =====================================================================
    // ROOT → вход в выбранный раздел
    // =====================================================================
    if (_mode == UiMode::NAV && _level == Level::ROOT) {
        enterSubmenu(MENU[_selected].target);
        return;
    }

    // =====================================================================
    // WIFI MENU (State / Scan)
    // =====================================================================
    if (_mode == UiMode::NAV && _level == Level::WIFI) {

        // -------------------------------------------------------------
        // 0 = State (ON / OFF)
        // → переходим в EDIT
        // -------------------------------------------------------------
        if (_subSelected == 0) {
            enterEdit();
            return;
        }

        // -------------------------------------------------------------
        // 1 = Scan
        // → запускаем scan
        // → переходим в WIFI_LIST
        // -------------------------------------------------------------
        if (_subSelected == 1) {
            _wifi.startScan();

            // сбрасываем позицию списка
            _wifiListSelected = 0;
            _wifiListTop      = 0;

            enterSubmenu(Level::WIFI_LIST);
            return;
        }
    }

    // =====================================================================
    // WIFI LIST
    // =====================================================================
    if (_mode == UiMode::NAV && _level == Level::WIFI_LIST) {

        /*
         * На этом этапе:
         *  - OK = Rescan
         *  - подключение к сети будет добавлено позже
         */
        _wifi.startScan();

        _wifiListSelected = 0;
        _wifiListTop      = 0;

        _dirty = true;
        return;
    }

    // =====================================================================
    // FALLBACK
    // =====================================================================
    // Для остальных пунктов — обычный вход в EDIT
    if (_mode == UiMode::NAV) {
        enterEdit();
    }
}

void SettingsScreen::onLongOk() {
    if (_mode == UiMode::EDIT)
        exitEdit(true);
}

void SettingsScreen::onShortBack() {
    _pressedBtn = HintBtn::BACK;
    _hintFlash  = 3;

    if (_mode == UiMode::EDIT) {
        exitEdit(false);
        return;
    }

    if (_level == Level::ROOT)
        _exitRequested = true;
    else
        exitSubmenu(true);
}

void SettingsScreen::onLongBack() {}

// ============================================================================
// SUBMENU CONTROL
// ============================================================================
void SettingsScreen::enterSubmenu(Level lvl) {
    _level = lvl;
    _mode  = UiMode::NAV;

    _subSelected = 0;
    _needFullClear = true;

    if (lvl == Level::WIFI) {
        // временное состояние для EDIT
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

// ============================================================================
// EXIT FLAGS
// ============================================================================
bool SettingsScreen::exitRequested() const {
    return _exitRequested;
}

void SettingsScreen::clearExitRequest() {
    _exitRequested = false;
}
void SettingsScreen::onThemeChanged() {
    _needFullClear = true;
    _dirty = true;
}