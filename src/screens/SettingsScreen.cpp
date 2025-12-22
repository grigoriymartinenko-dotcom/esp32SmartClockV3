#include "screens/SettingsScreen.h"
#include <Adafruit_GFX.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// PreferencesService — глобальный объект
// ============================================================================
extern PreferencesService prefs;

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

    _needFullClear  = true;
    _lastDrawnLevel = _level;

    _bar.setVisible(false);
}

// ============================================================================
// update
// ============================================================================
void SettingsScreen::update() {

    // мигание подсказок — ТОЛЬКО здесь (не в обработчиках кнопок)
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

    // Timezone — read-only: OK ничего не делает
    if (_level == Level::TIMEZONE) return;

    if (_mode == UiMode::NAV && _level == Level::ROOT) {
        enterSubmenu(MENU[_selected].target);
        return;
    }

    if (_mode == UiMode::NAV) enterEdit();
    else                      exitEdit(true);
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

void SettingsScreen::onLongOk()   {}
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
// NAVIGATION
// ============================================================================
void SettingsScreen::navLeft() {
    if (_level == Level::ROOT) {
        int n = sizeof(MENU) / sizeof(MENU[0]);
        _selected = (_selected + n - 1) % n;
        _dirty = true;
    }
}

void SettingsScreen::navRight() {
    if (_level == Level::ROOT) {
        int n = sizeof(MENU) / sizeof(MENU[0]);
        _selected = (_selected + 1) % n;
        _dirty = true;
    }
}

// ============================================================================
// EDIT CORE
// ============================================================================
void SettingsScreen::enterEdit() {
    // Timezone — read-only
    if (_level == Level::TIMEZONE) return;

    _mode = UiMode::EDIT;

    if (_level == Level::TIME) {
        _bakTimeMode = _tmpTimeMode;
    }

    if (_level == Level::NIGHT) {
        _bakMode = _night.mode();
        _tmpMode = _bakMode;
    }

    _dirty = true;
}

void SettingsScreen::exitEdit(bool apply) {
    if (!apply) {
        if (_level == Level::TIME)  _tmpTimeMode = _bakTimeMode;
        if (_level == Level::NIGHT) _tmpMode     = _bakMode;
    }

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

    if (_level == Level::NIGHT) {
        int v = (int)_tmpMode;
        _tmpMode = (NightService::Mode)((v + 1) % 3);
        _dirty = true;
    }
}

void SettingsScreen::editDec() {
    if (_level == Level::TIME) {
        int v = (int)_tmpTimeMode;
        _tmpTimeMode = (TimeService::Mode)((v + 3) % 4);
        _dirty = true;
        return;
    }

    if (_level == Level::NIGHT) {
        int v = (int)_tmpMode;
        _tmpMode = (NightService::Mode)((v + 2) % 3);
        _dirty = true;
    }
}

// ============================================================================
// SUBMENU
// ============================================================================
void SettingsScreen::enterSubmenu(Level lvl) {
    _level = lvl;
    _mode  = UiMode::NAV;

    // смена подменю => один полный клир области
    _needFullClear = true;

    if (lvl == Level::TIME) {
        _tmpTimeMode = (TimeService::Mode)prefs.timeSource();
        _bakTimeMode = _tmpTimeMode;
    }

    if (lvl == Level::NIGHT) {
        _tmpMode = _night.mode();
        _bakMode = _tmpMode;
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
        prefs.setNightMode(
            (_tmpMode == NightService::Mode::AUTO) ? NightModePref::AUTO :
            (_tmpMode == NightService::Mode::ON)   ? NightModePref::ON
                                                   : NightModePref::OFF
        );
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
void SettingsScreen::redrawAll() {
    const Theme& th = theme();

    int yStart = _layout.statusY() + _layout.statusH();
    int h      = _layout.buttonBarY() - yStart;

    // Полный клир — только когда реально нужен (переходы/тема)
    if (_needFullClear || _lastDrawnLevel != _level) {
        _tft.fillRect(0, yStart, _tft.width(), h, th.bg);
        _needFullClear  = false;
        _lastDrawnLevel = _level;
    }

    switch (_level) {
        case Level::ROOT:     drawRoot();     break;
        case Level::TIME:     drawTime();     break;
        case Level::NIGHT:    drawNight();    break;
        case Level::TIMEZONE: drawTimezone(); break;
    }

    drawButtonHints();
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

    int top    = 36;
    int bottom = _layout.buttonBarY();
    int count  = sizeof(MENU) / sizeof(MENU[0]);
    int rowH   = (bottom - top) / count;

    for (int i = 0; i < count; i++) {
        int y = top + i * rowH;

        // чистим только строку
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        bool sel = (i == _selected);

        _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print(MENU[i].label);
    }
}

// ============================================================================
// TIME
// ============================================================================
void SettingsScreen::drawTime() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(40, 8);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Time");

    _tft.fillRect(0, 40, _tft.width(), 24, th.bg);

    _tft.setTextSize(1);
    _tft.setCursor(20, 48);

    _tft.setTextColor(
        (_mode == UiMode::EDIT) ? ST77XX_RED : th.textPrimary,
        th.bg
    );

    const char* txt =
        (_tmpTimeMode == TimeService::AUTO)      ? "AUTO"  :
        (_tmpTimeMode == TimeService::RTC_ONLY) ? "RTC"   :
        (_tmpTimeMode == TimeService::NTP_ONLY) ? "NTP"   :
                                                   "LOCAL";

    _tft.print("Source: ");
    _tft.print(txt);
}

// ============================================================================
// NIGHT
// ============================================================================
void SettingsScreen::drawNight() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(24, 8);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Night mode");

    _tft.fillRect(0, 40, _tft.width(), 24, th.bg);

    _tft.setTextSize(1);
    _tft.setCursor(20, 48);

    _tft.setTextColor(
        (_mode == UiMode::EDIT) ? ST77XX_RED : th.textPrimary,
        th.bg
    );

    const char* txt =
        (_tmpMode == NightService::Mode::AUTO) ? "AUTO" :
        (_tmpMode == NightService::Mode::ON)   ? "ON"   :
                                                 "OFF";

    _tft.print("Mode: ");
    _tft.print(txt);
}

// ============================================================================
// TIMEZONE — РЕАЛЬНЫЕ ДАННЫЕ (READ-ONLY)
// ============================================================================
void SettingsScreen::drawTimezone() {
    const Theme& th = theme();

    int32_t gmt = prefs.tzGmtOffset();
    int32_t dst = prefs.tzDstOffset();

    char gmtBuf[8];
    char dstBuf[8];
    formatOffsetHM(gmt, gmtBuf, sizeof(gmtBuf));
    formatOffsetHM(dst, dstBuf, sizeof(dstBuf));

    _tft.setTextSize(2);
    _tft.setCursor(18, 8);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Timezone");

    _tft.fillRect(0, 34, _tft.width(), 48, th.bg);

    _tft.setTextSize(1);
    _tft.setTextColor(th.textPrimary, th.bg);

    _tft.setCursor(20, 40);
    _tft.print("UTC ");
    _tft.print(gmtBuf);

    _tft.setCursor(20, 56);
    _tft.print("DST ");
    _tft.print(dstBuf);
}

// ============================================================================
// BUTTON HINTS
// ============================================================================
void SettingsScreen::drawButtonHints() {
    const Theme& th = theme();
    int y0 = _layout.buttonBarY();

    _tft.fillRect(0, y0, _tft.width(), _tft.height() - y0, th.bg);

    _tft.setTextSize(1);
    _tft.setCursor(4, y0 + 4);

    auto col = [&](HintBtn b) {
        return (_hintFlash > 0 && _pressedBtn == b)
            ? ST77XX_GREEN
            : th.muted;
    };

    _tft.setTextColor(col(HintBtn::LEFT),  th.bg); _tft.print("< ");
    _tft.setTextColor(col(HintBtn::RIGHT), th.bg); _tft.print(">   ");
    _tft.setTextColor(col(HintBtn::OK),    th.bg); _tft.print("OK   ");
    _tft.setTextColor(col(HintBtn::BACK),  th.bg); _tft.print("BACK");
}