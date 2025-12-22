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
void SettingsScreen::redrawAll() {
    const Theme& th = theme();

    int yStart = 0;
    int h      = _layout.buttonBarY() - yStart;

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

    int top = 40;
    int rowH = 18;

    _tft.fillRect(0, top, _tft.width(), rowH, th.bg);

    bool sel = (_subSelected == 0);
    _tft.setTextSize(1);
    _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
    _tft.setCursor(12, top + 4);
    _tft.print(sel ? "> " : "  ");

    _tft.print("Source: ");

    uint16_t valCol = (_mode == UiMode::EDIT && sel) ? th.error
                                                     : (sel ? ST77XX_GREEN : th.textPrimary);
    _tft.setTextColor(valCol, th.bg);

    const char* txt =
        (_tmpTimeMode == TimeService::AUTO)      ? "AUTO"  :
        (_tmpTimeMode == TimeService::RTC_ONLY) ? "RTC"   :
        (_tmpTimeMode == TimeService::NTP_ONLY) ? "NTP"   :
                                                   "LOCAL";

    _tft.print(txt);
}

// ============================================================================
// NIGHT (3 items: Mode / Start / End)
// ============================================================================
void SettingsScreen::drawNight() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(24, 8);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Night mode");

    int top = 40;
    int rowH = 18;

    char buf[8];

    // --- Row 0: Mode ---
    {
        int y = top;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);
        bool sel = (_subSelected == 0);

        _tft.setTextSize(1);
        _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("Mode: ");

        uint16_t col =
            (_mode == UiMode::EDIT && sel) ? th.error :
            (sel ? ST77XX_GREEN : th.textPrimary);

        _tft.setTextColor(col, th.bg);

        _tft.print(
            _tmpMode == NightService::Mode::AUTO ? "AUTO" :
            _tmpMode == NightService::Mode::ON   ? "ON"   : "OFF"
        );
    }

    // --- Row 1: Start ---
    {
        int y = top + rowH;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);
        bool sel = (_subSelected == 1);

        snprintf(buf, sizeof(buf), "%02d:%02d", _tmpNightStart / 60, _tmpNightStart % 60);

        _tft.setTextSize(1);
        _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("Start: ");

        uint16_t col = (_mode == UiMode::EDIT && sel) ? th.error : th.textPrimary;
        _tft.setTextColor(col, th.bg);
        _tft.print(buf);
    }

    // --- Row 2: End ---
    {
        int y = top + rowH * 2;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);
        bool sel = (_subSelected == 2);

        snprintf(buf, sizeof(buf), "%02d:%02d", _tmpNightEnd / 60, _tmpNightEnd % 60);

        _tft.setTextSize(1);
        _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("End:   ");

        uint16_t col = (_mode == UiMode::EDIT && sel) ? th.error : th.textPrimary;
        _tft.setTextColor(col, th.bg);
        _tft.print(buf);
    }
}

// ============================================================================
// TIMEZONE
// ============================================================================
void SettingsScreen::drawTimezone() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setCursor(18, 8);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Timezone");

    int top = 40;
    int rowH = 18;

    int32_t utcSec = (_mode == UiMode::EDIT) ? _tmpTzSec : prefs.tzGmtOffset();
    int32_t dstSec = (_mode == UiMode::EDIT) ? s_tmpDstSec : prefs.tzDstOffset();

    char utcBuf[8];
    char dstBuf[8];
    formatOffsetHM(utcSec, utcBuf, sizeof(utcBuf));
    formatOffsetHM(dstSec, dstBuf, sizeof(dstBuf));

    // --- Row 0: UTC ---
    {
        int y = top + 0 * rowH;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        bool sel = (_subSelected == 0);

        _tft.setTextSize(1);
        _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("UTC ");

        uint16_t valCol =
            (_mode == UiMode::EDIT && sel)
                ? th.error
                : (sel ? ST77XX_GREEN : th.textPrimary);

        _tft.setTextColor(valCol, th.bg);
        _tft.print(utcBuf);
    }

    // --- Row 1: DST ---
    {
        int y = top + 1 * rowH;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        bool sel = (_subSelected == 1);

        _tft.setTextSize(1);
        _tft.setTextColor(sel ? ST77XX_GREEN : th.textPrimary, th.bg);
        _tft.setCursor(12, y + 4);
        _tft.print(sel ? "> " : "  ");
        _tft.print("DST ");

        uint16_t valCol =
            (_mode == UiMode::EDIT && sel)
                ? th.error
                : (sel ? ST77XX_GREEN : th.textPrimary);

        _tft.setTextColor(valCol, th.bg);
        _tft.print(dstBuf);
    }
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