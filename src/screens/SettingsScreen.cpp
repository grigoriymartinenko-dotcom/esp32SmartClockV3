#include "screens/SettingsScreen.h"

/*
 * SettingsScreen
 * --------------
 * Root Settings + Night Mode submenu
 * 4 hardware buttons UX
 */

// ================= ctor =================

SettingsScreen::SettingsScreen(
    Adafruit_ST7735& tft,
    ThemeService& themeService,
    LayoutService& layoutService,
    NightService& nightService,
    UiVersionService& uiVersion
)
    : Screen(themeService)
    , _tft(tft)
    , _layout(layoutService)
    , _bar(tft, themeService, layoutService)
    , _night(nightService)
    , _ui(uiVersion)
{
}

// ================= lifecycle =================

void SettingsScreen::begin() {
    _exitRequested = false;
    _level = Level::ROOT;

    _bar.setVisible(true);
    _bar.setActions(true, true, true, true);
    _bar.setHighlight(false, false, false, false);

    _dirty = true;
    redrawAll();
}

void SettingsScreen::update() {
    _bar.setHighlight(false, false, false, false);

    // мигание — только локальный redraw
    if (_level == Level::NIGHT_MODE && _editing) {
        _ui.bump(UiChannel::SCREEN);
        drawNightMenu();
    }

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

// ================= buttons (called by AppController) =================

void SettingsScreen::onLeft() {
    _bar.flash(ButtonBar::ButtonId::LEFT);

    if (_level == Level::ROOT) {
        if (_selected > 0) {
            _selected--;
            _dirty = true;
        }
    } else {
        nightLeft();
    }
}

void SettingsScreen::onRight() {
    _bar.flash(ButtonBar::ButtonId::RIGHT);

    if (_level == Level::ROOT) {
        if (_selected < ITEM_COUNT - 1) {
            _selected++;
            _dirty = true;
        }
    } else {
        nightRight();
    }
}

void SettingsScreen::onOk() {
    _bar.flash(ButtonBar::ButtonId::OK);

    if (_level == Level::NIGHT_MODE) {
        nightUp();
    }
}

void SettingsScreen::onBack() {
    _bar.flash(ButtonBar::ButtonId::BACK);

    if (_level == Level::NIGHT_MODE) {
        nightDown();
    }
}

void SettingsScreen::onOkLong() {
    _bar.flash(ButtonBar::ButtonId::OK);

    if (_level == Level::ROOT) {
        if (_selected == 2) { // Night mode
            enterNightMenu();
        }
    } else {
        nightEnter();
    }
}

void SettingsScreen::onBackLong() {
    _bar.flash(ButtonBar::ButtonId::BACK);

    if (_level == Level::ROOT) {
        _exitRequested = true;
    } else {
        nightExit();
    }
}

// ================= exit =================

bool SettingsScreen::exitRequested() const {
    return _exitRequested;
}

void SettingsScreen::clearExitRequest() {
    _exitRequested = false;
}

// ================= draw =================

void SettingsScreen::redrawAll() {
    const Theme& th = theme();
    _tft.fillScreen(th.bg);

    if (_level == Level::ROOT) {
        drawTitle();
        drawList();
    } else {
        drawNightMenu();
    }

    _bar.markDirty();
}

void SettingsScreen::drawTitle() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(20, 8);
    _tft.print("SETTINGS");
}

void SettingsScreen::drawList() {
    const Theme& th = theme();

    const int top = 36;
    const int bottom = _layout.buttonBarY();
    const int h = bottom - top;
    const int rowH = h / ITEM_COUNT;

    for (int i = 0; i < ITEM_COUNT; i++) {
        int y = top + i * rowH;

        bool sel = (i == _selected);
        uint16_t color = sel ? th.accent : th.textPrimary;

        _tft.setTextSize(1);
        _tft.setTextColor(color, th.bg);
        _tft.setCursor(12, y + (rowH - 8) / 2);

        if (i == 0) _tft.print("Wi-Fi");
        if (i == 1) _tft.print("Timezone");
        if (i == 2) _tft.print("Night mode");
        if (i == 3) _tft.print("Future 1");
        if (i == 4) _tft.print("Future 2");
        if (i == 5) _tft.print("About");
    }
}

// ================= night submenu =================

void SettingsScreen::enterNightMenu() {
    _level = Level::NIGHT_MODE;
    _editing = false;
    _nightField = NightField::MODE;
    _timePart = TimePart::HH;

    _tmpMode = _night.mode();
    _tmpStartMin = _night.autoStart();
    _tmpEndMin   = _night.autoEnd();

    _dirty = true;
}

void SettingsScreen::exitNightMenu(bool apply) {
    if (apply) {
        _night.setMode(_tmpMode);
        _night.setAutoRange(_tmpStartMin, _tmpEndMin);
    }

    _level = Level::ROOT;
    _editing = false;
    _dirty = true;
}

void SettingsScreen::nightEnter() {
    _editing = !_editing;
    if (_editing) _timePart = TimePart::HH;
    _dirty = true;
}

void SettingsScreen::nightExit() {
    if (_editing) {
        _editing = false;
        _dirty = true;
    } else {
        exitNightMenu(true);
    }
}

void SettingsScreen::nightLeft() {
    if (_editing) {
        // редактирование времени: HH ↔ MM
        if (_nightField != NightField::MODE) {
            _timePart = TimePart::HH;
        }
    } else {
        // навигация по пунктам ПО КРУГУ
        if (_nightField == NightField::MODE) {
            _nightField = NightField::END;
        } else {
            _nightField = static_cast<NightField>((int)_nightField - 1);
        }
    }
    _dirty = true;
}

void SettingsScreen::nightRight() {
    if (_editing) {
        // редактирование времени: HH ↔ MM
        if (_nightField != NightField::MODE) {
            _timePart = TimePart::MM;
        }
    } else {
        // навигация по пунктам ПО КРУГУ
        if (_nightField == NightField::END) {
            _nightField = NightField::MODE;
        } else {
            _nightField = static_cast<NightField>((int)_nightField + 1);
        }
    }
    _dirty = true;
}

void SettingsScreen::nightUp() {
    if (!_editing) return;

    if (_nightField == NightField::MODE) {
        _tmpMode = (NightService::Mode)(((int)_tmpMode + 1) % 3);
    } else {
        int* v = (_nightField == NightField::START) ? &_tmpStartMin : &_tmpEndMin;
        int hh = *v / 60;
        int mm = *v % 60;

        if (_timePart == TimePart::HH) hh = (hh + 1) % 24;
        else mm = (mm + 1) % 60;

        *v = hh * 60 + mm;
    }
}

void SettingsScreen::nightDown() {
    if (!_editing) return;

    if (_nightField == NightField::MODE) {
        int m = (int)_tmpMode - 1;
        if (m < 0) m = 2;
        _tmpMode = (NightService::Mode)m;
    } else {
        int* v = (_nightField == NightField::START) ? &_tmpStartMin : &_tmpEndMin;
        int hh = *v / 60;
        int mm = *v % 60;

        if (_timePart == TimePart::HH) hh = (hh + 23) % 24;
        else mm = (mm + 59) % 60;

        *v = hh * 60 + mm;
    }
}

void SettingsScreen::drawNightMenu() {
    const Theme& th = theme();

    // медленное мигание
    bool blinkOn = ((_ui.version(UiChannel::SCREEN) / 8) % 2) == 0;

    const uint16_t editColor = ST77XX_WHITE;   // редактирование
    const uint16_t navColor  = th.accent;      // навигация
    const uint16_t normColor = th.textPrimary; // обычный текст

    // ===== TITLE =====
    _tft.setTextSize(2);
    _tft.setTextColor(normColor, th.bg);
    _tft.setCursor(20, 8);
    _tft.print("Night Mode");

    _tft.setTextSize(1);

    int y = 40;
    const int row = 18;
    const int underlineY = y + 7; // базовая линия подчёркивания

    auto rowColor = [&](NightField f) -> uint16_t {
        if (_nightField == f) {
            return _editing ? editColor : navColor;
        }
        return normColor;
    };

    // ================= MODE =================
    _tft.setCursor(12, y);
    _tft.setTextColor(rowColor(NightField::MODE), th.bg);
    _tft.print("Mode: ");

    int modeX = _tft.getCursorX();

    if (!_editing || _nightField != NightField::MODE || blinkOn) {
        if (_tmpMode == NightService::Mode::AUTO) _tft.print("AUTO");
        if (_tmpMode == NightService::Mode::ON)   _tft.print("ON  ");
        if (_tmpMode == NightService::Mode::OFF)  _tft.print("OFF ");
    }

    if (_editing && _nightField == NightField::MODE) {
        int w = 4 * 6; // ширина AUTO/ON/OFF
        _tft.drawFastHLine(modeX, underlineY, w, editColor);
    }

    // ================= START =================
    y += row;
    int underlineY2 = y + 7;

    _tft.setCursor(12, y);
    _tft.setTextColor(rowColor(NightField::START), th.bg);
    _tft.print("Start: ");

    int sh = _tmpStartMin / 60;
    int sm = _tmpStartMin % 60;

    int xHH = _tft.getCursorX();

    if (!_editing || _nightField != NightField::START || blinkOn || _timePart != TimePart::HH) {
        if (sh < 10) _tft.print('0');
        _tft.print(sh);
    } else {
        _tft.print("  ");
    }

    _tft.print(':');
    int xMM = _tft.getCursorX();

    if (!_editing || _nightField != NightField::START || blinkOn || _timePart != TimePart::MM) {
        if (sm < 10) _tft.print('0');
        _tft.print(sm);
    } else {
        _tft.print("  ");
    }

    if (_editing && _nightField == NightField::START) {
        int ux = (_timePart == TimePart::HH) ? xHH : xMM;
        _tft.drawFastHLine(ux, underlineY2, 12, editColor); // 2 цифры
    }

    // ================= END =================
    y += row;
    int underlineY3 = y + 7;

    _tft.setCursor(12, y);
    _tft.setTextColor(rowColor(NightField::END), th.bg);
    _tft.print("End:   ");

    int eh = _tmpEndMin / 60;
    int em = _tmpEndMin % 60;

    xHH = _tft.getCursorX();

    if (!_editing || _nightField != NightField::END || blinkOn || _timePart != TimePart::HH) {
        if (eh < 10) _tft.print('0');
        _tft.print(eh);
    } else {
        _tft.print("  ");
    }

    _tft.print(':');
    xMM = _tft.getCursorX();

    if (!_editing || _nightField != NightField::END || blinkOn || _timePart != TimePart::MM) {
        if (em < 10) _tft.print('0');
        _tft.print(em);
    } else {
        _tft.print("  ");
    }

    if (_editing && _nightField == NightField::END) {
        int ux = (_timePart == TimePart::HH) ? xHH : xMM;
        _tft.drawFastHLine(ux, underlineY3, 12, editColor);
    }
}