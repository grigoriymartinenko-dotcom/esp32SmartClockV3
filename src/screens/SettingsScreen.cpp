#include "screens/SettingsScreen.h"

// ============================================================================
// PreferencesService — глобальный объект, созданный в main.cpp
// ============================================================================
extern PreferencesService prefs;

// ============================================================================
// static constexpr arrays
// ============================================================================
constexpr SettingsScreen::MenuItem SettingsScreen::MENU[];
constexpr SettingsScreen::TzItem   SettingsScreen::TZ_LIST[];

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
{
}

// ============================================================================
// begin
// ============================================================================
void SettingsScreen::begin() {
    _exitRequested = false;
    _level = Level::ROOT;
    _mode  = UiMode::NAV;
    _dirty = true;

    _bar.setVisible(true);
    _bar.setActions(true, true, true, true);
    _bar.markDirty();
}

// ============================================================================
// update
// ============================================================================
void SettingsScreen::update() {
    if (_dirty) {
        redrawAll();
        _dirty = false;
    }
    _bar.update();
}

void SettingsScreen::onThemeChanged() {
    _bar.markDirty();
    _dirty = true;
}

// ============================================================================
// BUTTONS
// ============================================================================
void SettingsScreen::onShortLeft() {
    if (_mode == UiMode::NAV) { navLeft(); return; }
}

void SettingsScreen::onShortRight() {
    if (_mode == UiMode::NAV) { navRight(); return; }
}

void SettingsScreen::onShortOk() {
    if (_mode == UiMode::EDIT) editInc();
}

void SettingsScreen::onShortBack() {
    if (_mode == UiMode::EDIT) editDec();
}

void SettingsScreen::onLongOk() {
    if (_level == Level::ROOT) {
        enterSubmenu(MENU[_selected].target);
        return;
    }

    // SUBMENU
    if (_mode == UiMode::NAV) enterEdit();
    else                      exitEdit(true);
}

void SettingsScreen::onLongBack() {
    if (_level == Level::ROOT) {
        _exitRequested = true;
        return;
    }

    if (_mode == UiMode::EDIT) exitEdit(false);
    else                       exitSubmenu(true);
}

// ============================================================================
// Exit flags
// ============================================================================
bool SettingsScreen::exitRequested() const {
    return _exitRequested;
}

void SettingsScreen::clearExitRequest() {
    _exitRequested = false;
}

// ============================================================================
// NAV
// ============================================================================
void SettingsScreen::navLeft() {
    if (_level == Level::ROOT) {
        const int n = sizeof(MENU) / sizeof(MENU[0]);
        _selected = (_selected + n - 1) % n;
        _dirty = true;
        return;
    }

    if (_level == Level::TIMEZONE) {
        _tzField = (_tzField == TzField::ZONE) ? TzField::DST : TzField::ZONE;
        _dirty = true;
        return;
    }
}

void SettingsScreen::navRight() {
    if (_level == Level::ROOT) {
        const int n = sizeof(MENU) / sizeof(MENU[0]);
        _selected = (_selected + 1) % n;
        _dirty = true;
        return;
    }

    if (_level == Level::TIMEZONE) {
        _tzField = (_tzField == TzField::DST) ? TzField::ZONE : TzField::DST;
        _dirty = true;
        return;
    }
}

// ============================================================================
// EDIT CORE  ✅ ВАЖНО: ЭТИХ ФУНКЦИЙ НЕ ХВАТАЛО
// ============================================================================
void SettingsScreen::enterEdit() {
    _mode = UiMode::EDIT;
    _dirty = true;
}

void SettingsScreen::exitEdit(bool apply) {
    // Для Timezone:
    // apply=false -> rollback
    // apply=true  -> оставить текущие значения

    if (!apply) {
        if (_level == Level::TIMEZONE) {
            _tzIndex  = _bakTzIndex;
            _dstAuto  = _bakDstAuto;
        }
    } else {
        if (_level == Level::TIMEZONE) {
            _bakTzIndex = _tzIndex;
            _bakDstAuto = _dstAuto;
        }
    }

    _mode = UiMode::NAV;
    _dirty = true;
}

// ============================================================================
// EDIT ACTIONS
// ============================================================================
void SettingsScreen::editInc() {
    if (_level == Level::TIMEZONE) {
        if (_tzField == TzField::ZONE) {
            int n = sizeof(TZ_LIST) / sizeof(TZ_LIST[0]);
            _tzIndex = (_tzIndex + 1) % n;
        } else {
            _dstAuto = !_dstAuto;
        }
        _dirty = true;
    }
}

void SettingsScreen::editDec() {
    editInc();
}

// ============================================================================
// SUBMENU
// ============================================================================
void SettingsScreen::enterSubmenu(Level lvl) {
    if (lvl == Level::ROOT) return;

    _level = lvl;
    _mode  = UiMode::NAV;

    if (lvl == Level::TIMEZONE) {
        long gmt = prefs.tzGmtOffset();
        int  dst = prefs.tzDstOffset();

        _dstAuto = (dst != 0);
        _bakDstAuto = _dstAuto;

        int n = sizeof(TZ_LIST) / sizeof(TZ_LIST[0]);
        for (int i = 0; i < n; i++) {
            if (TZ_LIST[i].gmtOffset == gmt) {
                _tzIndex = i;
                break;
            }
        }
        _bakTzIndex = _tzIndex;
        _tzField = TzField::ZONE;
    }

    _dirty = true;
}

void SettingsScreen::exitSubmenu(bool apply) {
    if (_level == Level::TIMEZONE && apply) {
        const TzItem& tz = TZ_LIST[_tzIndex];
        int dstOffset = _dstAuto ? tz.dstOffset : 0;

        prefs.setTimezone(tz.gmtOffset, dstOffset);
        prefs.save();

        _time.setTimezone(tz.gmtOffset, dstOffset);
    }

    _level = Level::ROOT;
    _mode  = UiMode::NAV;
    _dirty = true;
}

// ============================================================================
// DRAW
// ============================================================================
void SettingsScreen::redrawAll() {
    const Theme& th = theme();
    _tft.fillScreen(th.bg);

    if (_level == Level::ROOT)       drawRoot();
    else if (_level == Level::NIGHT) drawNight();
    else                             drawTimezone();

    _bar.markDirty();
}

void SettingsScreen::drawRoot() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(20, 8);
    _tft.print("SETTINGS");

    _tft.setTextSize(1);

    const int top = 36;
    const int bottom = _layout.buttonBarY();
    const int count = sizeof(MENU) / sizeof(MENU[0]);
    const int rowH = (bottom - top) / count;

    for (int i = 0; i < count; i++) {
        uint16_t color = (i == _selected) ? th.accent : th.textPrimary;
        _tft.setTextColor(color, th.bg);
        _tft.setCursor(12, top + i * rowH + 4);
        _tft.print(MENU[i].label);
    }
}

void SettingsScreen::drawNight() {
    _tft.setTextSize(1);
    _tft.setCursor(12, 40);
    _tft.print("Night settings...");
}

void SettingsScreen::drawTimezone() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(18, 8);
    _tft.print("Timezone");

    _tft.setTextSize(1);

    uint16_t zoneColor =
        (_tzField == TzField::ZONE)
            ? (_mode == UiMode::EDIT ? ST77XX_WHITE : th.accent)
            : th.textPrimary;

    uint16_t dstColor =
        (_tzField == TzField::DST)
            ? (_mode == UiMode::EDIT ? ST77XX_WHITE : th.accent)
            : th.textPrimary;

    _tft.setCursor(12, 44);
    _tft.setTextColor(zoneColor, th.bg);
    _tft.print("Zone: ");
    _tft.print(TZ_LIST[_tzIndex].name);

    _tft.setCursor(12, 64);
    _tft.setTextColor(dstColor, th.bg);
    _tft.print("DST:  ");
    _tft.print(_dstAuto ? "AUTO" : "OFF");
}