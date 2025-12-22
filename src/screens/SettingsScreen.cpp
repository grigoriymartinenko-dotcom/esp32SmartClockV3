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
{}

// ============================================================================
// begin
// ============================================================================
void SettingsScreen::begin() {
    _exitRequested = false;
    _level = Level::ROOT;
    _mode  = UiMode::NAV;
    _dirty = true;

    // ButtonBar скрываем — он рисует дефолтные подписи "<- OK -> Back"
    _bar.setVisible(false);
}

// ============================================================================
// update
// ============================================================================
void SettingsScreen::update() {
    if (_dirty) {
        _dirty = false;   // ВАЖНО: сброс ДО redraw
        redrawAll();
    }
}

void SettingsScreen::onThemeChanged() {
    _dirty = true;
}

// ============================================================================
// BUTTONS (фиксируем кнопку + запускаем мигание)
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
    } else {
        exitEdit(true);
    }
}

void SettingsScreen::onShortBack() {
    _pressedBtn = HintBtn::BACK;
    _hintFlash  = 3;

    if (_mode == UiMode::EDIT) {
        exitEdit(false);
        return;
    }

    if (_level == Level::ROOT) {
        _exitRequested = true;
    } else {
        exitSubmenu(true);
    }
}

void SettingsScreen::onLongOk()   {}
void SettingsScreen::onLongBack() {}

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
        int n = sizeof(MENU) / sizeof(MENU[0]);
        _selected = (_selected + n - 1) % n;
        _dirty = true;
        return;
    }

    if (_level == Level::TIMEZONE) {
        _tzField = (_tzField == TzField::ZONE)
            ? TzField::DST
            : TzField::ZONE;
        _dirty = true;
    }
}

void SettingsScreen::navRight() {
    if (_level == Level::ROOT) {
        int n = sizeof(MENU) / sizeof(MENU[0]);
        _selected = (_selected + 1) % n;
        _dirty = true;
        return;
    }

    if (_level == Level::TIMEZONE) {
        _tzField = (_tzField == TzField::DST)
            ? TzField::ZONE
            : TzField::DST;
        _dirty = true;
    }
}

// ============================================================================
// EDIT CORE
// ============================================================================
void SettingsScreen::enterEdit() {
    _mode = UiMode::EDIT;

    if (_level == Level::TIMEZONE) {
        _bakTzIndex = _tzIndex;
        _bakDstAuto = _dstAuto;
    }

    if (_level == Level::NIGHT) {
        _bakMode = _night.mode();
        _tmpMode = _bakMode;
    }

    _dirty = true;
}

void SettingsScreen::exitEdit(bool apply) {
    if (!apply) {
        if (_level == Level::TIMEZONE) {
            _tzIndex = _bakTzIndex;
            _dstAuto = _bakDstAuto;
        }
        if (_level == Level::NIGHT) {
            _tmpMode = _bakMode;
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
            _dstAuto = (_dstAuto + 1) % 3;
        }
        _dirty = true;
        return;
    }

    if (_level == Level::NIGHT) {
        int m = static_cast<int>(_tmpMode);
        m = (m + 1) % 3;
        _tmpMode = static_cast<NightService::Mode>(m);
        _dirty = true;
    }
}

void SettingsScreen::editDec() {

    if (_level == Level::TIMEZONE) {
        if (_tzField == TzField::ZONE) {
            int n = sizeof(TZ_LIST) / sizeof(TZ_LIST[0]);
            _tzIndex = (_tzIndex + n - 1) % n;
        } else {
            _dstAuto = (_dstAuto + 2) % 3;
        }
        _dirty = true;
        return;
    }

    if (_level == Level::NIGHT) {
        int m = static_cast<int>(_tmpMode);
        m = (m + 2) % 3;
        _tmpMode = static_cast<NightService::Mode>(m);
        _dirty = true;
    }
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

        _dstAuto = (dst == 0) ? 2 : 0;
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

    if (lvl == Level::NIGHT) {
        _tmpMode = _night.mode();
        _bakMode = _tmpMode;
    }

    _dirty = true;
}

void SettingsScreen::exitSubmenu(bool apply) {

    if (_level == Level::TIMEZONE && apply) {
        const TzItem& tz = TZ_LIST[_tzIndex];
        int dstOffset = (_dstAuto == 2) ? 0 : tz.dstOffset;

        prefs.setTimezone(tz.gmtOffset, dstOffset);
        prefs.save();
        _time.setTimezone(tz.gmtOffset, dstOffset);
    }

    if (_level == Level::NIGHT && apply) {
        _night.setMode(_tmpMode);

        NightModePref pref =
            (_tmpMode == NightService::Mode::AUTO) ? NightModePref::AUTO :
            (_tmpMode == NightService::Mode::ON)   ? NightModePref::ON
                                                   : NightModePref::OFF;

        prefs.setNightMode(pref);
        prefs.save();
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

    // Основной экран перерисовываем ТОЛЬКО если это не кадр мигания
    if (_hintFlash == 0) {
        _tft.fillRect(
            0,
            0,
            _tft.width(),
            _layout.buttonBarY(),
            th.bg
        );

        if (_level == Level::ROOT)       drawRoot();
        else if (_level == Level::NIGHT) drawNight();
        else                             drawTimezone();
    }

    // Подсказки кнопок — всегда
    drawButtonHints();

    // 2–3 кадра мигания
    if (_hintFlash > 0) {
        _hintFlash--;
        _dirty = true;
    }
}

// ============================================================================
// ROOT
// ============================================================================
void SettingsScreen::drawRoot() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(20, 8);
    _tft.print("SETTINGS");

    _tft.setTextSize(1);

    int top = 36;
    int bottom = _layout.buttonBarY();
    int count = sizeof(MENU) / sizeof(MENU[0]);
    int rowH = (bottom - top) / count;

    for (int i = 0; i < count; i++) {
        bool sel = (i == _selected);
        uint16_t color = sel ? th.accent : th.textPrimary;

        _tft.setTextColor(color, th.bg);
        _tft.setCursor(12, top + i * rowH + 4);

        _tft.print(sel ? "> " : "  ");
        _tft.print(MENU[i].label);
    }
}

// ============================================================================
// NIGHT
// ============================================================================
void SettingsScreen::drawNight() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(24, 8);
    _tft.print("Night mode");

    _tft.setTextSize(1);
    _tft.setCursor(12, 48);

    uint16_t c = (_mode == UiMode::EDIT) ? ST77XX_WHITE : th.accent;
    _tft.setTextColor(c, th.bg);

    const char* txt =
        (_tmpMode == NightService::Mode::AUTO) ? "AUTO" :
        (_tmpMode == NightService::Mode::ON)   ? "ON"   : "OFF";

    _tft.print("Mode: ");
    _tft.print(txt);
}

// ============================================================================
// TIMEZONE
// ============================================================================
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
    _tft.print(
        (_dstAuto == 0) ? "AUTO" :
        (_dstAuto == 1) ? "ON"   : "OFF"
    );
}

// ============================================================================
// BUTTON HINTS (мигание 2–3 кадра, без мигания экрана)
// ============================================================================
void SettingsScreen::drawButtonHints() {
    const Theme& th = theme();
    int y0 = _layout.buttonBarY();

    // очищаем ТОЛЬКО область подсказок
    _tft.fillRect(
        0,
        y0,
        _tft.width(),
        _tft.height() - y0,
        th.bg
    );

    int y = y0 + 4;

    _tft.setTextSize(1);
    _tft.setCursor(4, y);

    auto col = [&](HintBtn b) {
        return (_hintFlash > 0 && _pressedBtn == b)
            ? th.accent
            : th.muted;
    };

    _tft.setTextColor(col(HintBtn::LEFT), th.bg);
    _tft.print("< ");

    _tft.setTextColor(col(HintBtn::RIGHT), th.bg);
    _tft.print(">   ");

    if (_level == Level::ROOT) {
        _tft.setTextColor(col(HintBtn::OK), th.bg);
        _tft.print("OK:Select   ");
        _tft.setTextColor(col(HintBtn::BACK), th.bg);
        _tft.print("BACK");
    }
    else if (_mode == UiMode::NAV) {
        _tft.setTextColor(col(HintBtn::OK), th.bg);
        _tft.print("OK:Edit     ");
        _tft.setTextColor(col(HintBtn::BACK), th.bg);
        _tft.print("BACK");
    }
    else {
        _tft.setTextColor(col(HintBtn::OK), th.bg);
        _tft.print("OK:Apply    ");
        _tft.setTextColor(col(HintBtn::BACK), th.bg);
        _tft.print("BACK:Cancel");
    }
}