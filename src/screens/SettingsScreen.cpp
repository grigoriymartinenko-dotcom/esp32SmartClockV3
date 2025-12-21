#include "screens/SettingsScreen.h"
#include "services/PreferencesService.h"

// ============================================================================
// ВАЖНО:
// PreferencesService создан в main.cpp как глобальный объект:
//     PreferencesService prefs;
// Здесь мы просто используем его через extern.
// ============================================================================
extern PreferencesService prefs;

// ============================================================================
// ВАЖНО ДЛЯ C++:
// В SettingsScreen.h TZ_LIST объявлен как static constexpr массив.
// Чтобы линкер/компилятор были счастливы, нужно ОДНО внешнее определение.
// Это оно:
// ============================================================================
constexpr SettingsScreen::TzItem SettingsScreen::TZ_LIST[];

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
// begin(): вызывается при заходе на экран Settings
// ============================================================================
void SettingsScreen::begin() {
    _exitRequested = false;
    _level = Level::ROOT;

    // ButtonBar (нижняя панель подсказок/индикации кнопок)
    _bar.setVisible(true);
    _bar.setActions(true, true, true, true);
    _bar.setHighlight(false, false, false, false);

    _dirty = true;
    redrawAll();
}

// ============================================================================
// update(): вызывается каждый loop() когда SettingsScreen активен
// ============================================================================
void SettingsScreen::update() {
    // каждый кадр сбрасываем подсветки кнопок,
    // flash() потом кратко включает её
    _bar.setHighlight(false, false, false, false);

    // Моргание/перерисовка в режимах редактирования
    // (мы просто "пинаем" UiChannel::SCREEN)
    if (_level == Level::NIGHT_MODE && _editing) {
        _ui.bump(UiChannel::SCREEN);
        drawNightMenu();
    }

    if (_level == Level::TIMEZONE && _tzEditing) {
        _ui.bump(UiChannel::SCREEN);
        drawTimezoneMenu();
    }

    // Полная перерисовка экрана по флагу
    if (_dirty) {
        redrawAll();
        _dirty = false;
    }

    // Обновляем нижнюю панель
    _bar.update();
}

// ============================================================================
// Когда тема изменилась — перерисуем
// ============================================================================
void SettingsScreen::onThemeChanged() {
    _bar.markDirty();
    _dirty = true;
}

// ============================================================================
// Кнопки: логика роутинга по текущему уровню (ROOT / NIGHT_MODE / TIMEZONE)
// ============================================================================
void SettingsScreen::onLeft() {
    _bar.flash(ButtonBar::ButtonId::LEFT);

    if (_level == Level::ROOT) {
        _selected = (_selected + ITEM_COUNT - 1) % ITEM_COUNT;
        _dirty = true;
    } else if (_level == Level::NIGHT_MODE) {
        nightLeft();
    } else { // TIMEZONE
        tzLeft();
    }
}

void SettingsScreen::onRight() {
    _bar.flash(ButtonBar::ButtonId::RIGHT);

    if (_level == Level::ROOT) {
        _selected = (_selected + 1) % ITEM_COUNT;
        _dirty = true;
    } else if (_level == Level::NIGHT_MODE) {
        nightRight();
    } else { // TIMEZONE
        tzRight();
    }
}

void SettingsScreen::onOk() {
    _bar.flash(ButtonBar::ButtonId::OK);

    // В подменю OK/Back двигают значения вверх/вниз
    if (_level == Level::NIGHT_MODE) nightUp();
    if (_level == Level::TIMEZONE)   tzUp();
}

void SettingsScreen::onBack() {
    _bar.flash(ButtonBar::ButtonId::BACK);

    if (_level == Level::NIGHT_MODE) nightDown();
    if (_level == Level::TIMEZONE)   tzDown();
}

void SettingsScreen::onOkLong() {
    _bar.flash(ButtonBar::ButtonId::OK);

    // LONG OK:
    // - в ROOT: вход в подменю (timezone/night)
    // - в NIGHT_MODE: toggle edit mode
    // - в TIMEZONE: toggle edit mode
    if (_level == Level::ROOT) {
        if (_selected == 1) enterTimezoneMenu();
        if (_selected == 2) enterNightMenu();
    } else if (_level == Level::NIGHT_MODE) {
        nightEnter();
    } else { // TIMEZONE
        tzEnter();
    }
}

void SettingsScreen::onBackLong() {
    _bar.flash(ButtonBar::ButtonId::BACK);

    // LONG BACK:
    // - в ROOT: выход из SettingsScreen (флаг)
    // - в подменю: назад (с apply)
    if (_level == Level::ROOT) {
        _exitRequested = true;
    } else if (_level == Level::NIGHT_MODE) {
        nightExit();
    } else { // TIMEZONE
        tzExit();
    }
}

// ============================================================================
// Exit flags (для AppController)
// ============================================================================
bool SettingsScreen::exitRequested() const {
    return _exitRequested;
}

void SettingsScreen::clearExitRequest() {
    _exitRequested = false;
}

// ============================================================================
// Draw root dispatcher
// ============================================================================
void SettingsScreen::redrawAll() {
    const Theme& th = theme();
    _tft.fillScreen(th.bg);

    if (_level == Level::ROOT) {
        drawTitle();
        drawList();
    } else if (_level == Level::NIGHT_MODE) {
        drawNightMenu();
    } else { // TIMEZONE
        drawTimezoneMenu();
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
    const int rowH = (bottom - top) / ITEM_COUNT;

    for (int i = 0; i < ITEM_COUNT; i++) {
        int y = top + i * rowH;
        uint16_t color = (i == _selected) ? th.accent : th.textPrimary;

        _tft.setTextSize(1);
        _tft.setTextColor(color, th.bg);
        _tft.setCursor(12, y + 4);

        if (i == 0) _tft.print("Wi-Fi");
        if (i == 1) _tft.print("Timezone");
        if (i == 2) _tft.print("Night mode");
        if (i == 3) _tft.print("Future 1");
        if (i == 4) _tft.print("Future 2");
        if (i == 5) _tft.print("About");
    }
}

// ============================================================================
// NIGHT MENU
// ============================================================================

void SettingsScreen::enterNightMenu() {
    _level = Level::NIGHT_MODE;
    _editing = false;
    _nightField = NightField::MODE;
    _timePart = TimePart::HH;

    // Берём текущие значения сервиса как стартовые для редактирования
    _tmpMode = _night.mode();
    _tmpStartMin = _night.autoStart();
    _tmpEndMin   = _night.autoEnd();

    _dirty = true;
}

void SettingsScreen::exitNightMenu(bool apply) {
    if (apply) {
        // Применяем в NightService
        _night.setMode(_tmpMode);
        _night.setAutoRange(_tmpStartMin, _tmpEndMin);

        // Сохраняем в EEPROM
        prefs.setNightMode(
            _tmpMode == NightService::Mode::AUTO ? NightModePref::AUTO :
            _tmpMode == NightService::Mode::ON   ? NightModePref::ON   :
                                                   NightModePref::OFF
        );
        prefs.setNightRange(_tmpStartMin, _tmpEndMin);
        prefs.save();
    }

    _level = Level::ROOT;
    _editing = false;
    _dirty = true;
}

void SettingsScreen::nightEnter() {
    // Включаем/выключаем режим редактирования
    _editing = !_editing;
    if (_editing) _timePart = TimePart::HH;
    _dirty = true;
}

void SettingsScreen::nightExit() {
    // Если редактировали — просто выходим из edit.
    // Если не редактировали — выходим из меню и применяем.
    if (_editing) {
        _editing = false;
        _dirty = true;
    } else {
        exitNightMenu(true);
    }
}

void SettingsScreen::nightLeft() {
    // Навигация по полям MODE/START/END
    // Если editing времени — left делает HH активным
    if (_editing && _nightField != NightField::MODE) {
        _timePart = TimePart::HH;
    } else if (!_editing) {
        _nightField = (_nightField == NightField::MODE)
            ? NightField::END
            : (NightField)((int)_nightField - 1);
    }
    _dirty = true;
}

void SettingsScreen::nightRight() {
    // Аналогично right: если editing времени — MM активным
    if (_editing && _nightField != NightField::MODE) {
        _timePart = TimePart::MM;
    } else if (!_editing) {
        _nightField = (_nightField == NightField::END)
            ? NightField::MODE
            : (NightField)((int)_nightField + 1);
    }
    _dirty = true;
}

void SettingsScreen::nightUp() {
    if (!_editing) return;

    if (_nightField == NightField::MODE) {
        // AUTO -> ON -> OFF
        _tmpMode = (NightService::Mode)(((int)_tmpMode + 1) % 3);
        return;
    }

    // START/END: редактируем часы/минуты
    int* v = (_nightField == NightField::START) ? &_tmpStartMin : &_tmpEndMin;
    int hh = *v / 60;
    int mm = *v % 60;

    if (_timePart == TimePart::HH) hh = (hh + 1) % 24;
    else mm = (mm + 1) % 60;

    *v = hh * 60 + mm;
}

void SettingsScreen::nightDown() {
    if (!_editing) return;

    if (_nightField == NightField::MODE) {
        int m = (int)_tmpMode - 1;
        if (m < 0) m = 2;
        _tmpMode = (NightService::Mode)m;
        return;
    }

    int* v = (_nightField == NightField::START) ? &_tmpStartMin : &_tmpEndMin;
    int hh = *v / 60;
    int mm = *v % 60;

    if (_timePart == TimePart::HH) hh = (hh + 23) % 24;
    else mm = (mm + 59) % 60;

    *v = hh * 60 + mm;
}

void SettingsScreen::drawNightMenu() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(20, 8);
    _tft.print("Night Mode");

    _tft.setTextSize(1);

    int y = 40;
    const int row = 18;

    // Цвет строки: если выделено поле — accent,
    // если editing — белым
    auto rowColor = [&](NightField f) -> uint16_t {
        if (_nightField == f) return _editing ? ST77XX_WHITE : th.accent;
        return th.textPrimary;
    };

    _tft.setCursor(12, y);
    _tft.setTextColor(rowColor(NightField::MODE), th.bg);
    _tft.print("Mode: ");
    _tft.print(
        _tmpMode == NightService::Mode::AUTO ? "AUTO" :
        _tmpMode == NightService::Mode::ON   ? "ON"   : "OFF"
    );

    y += row;
    _tft.setCursor(12, y);
    _tft.setTextColor(rowColor(NightField::START), th.bg);
    _tft.printf("Start: %02d:%02d", _tmpStartMin / 60, _tmpStartMin % 60);

    y += row;
    _tft.setCursor(12, y);
    _tft.setTextColor(rowColor(NightField::END), th.bg);
    _tft.printf("End:   %02d:%02d", _tmpEndMin / 60, _tmpEndMin % 60);
}

// ============================================================================
// TIMEZONE MENU
// ============================================================================

void SettingsScreen::enterTimezoneMenu() {
    _level = Level::TIMEZONE;
    _tzEditing = false;

    // ------------------------------------------------------------------------
    // КЛЮЧЕВОЕ:
    // при входе мы читаем сохранённый timezone из EEPROM (prefs)
    // и выставляем _tzIndex на соответствующий элемент массива TZ_LIST
    // ------------------------------------------------------------------------
    long savedGmt = (long)prefs.tzGmtOffset();
    int  savedDst = (int)prefs.tzDstOffset();

    int n = sizeof(TZ_LIST) / sizeof(TZ_LIST[0]);
    for (int i = 0; i < n; i++) {
        if (TZ_LIST[i].gmtOffset == savedGmt &&
            TZ_LIST[i].dstOffset == savedDst) {
            _tzIndex = i;
            break;
        }
    }

    _dirty = true;
}

void SettingsScreen::exitTimezoneMenu(bool apply) {
    if (apply) {
        const TzItem& tz = TZ_LIST[_tzIndex];

        // --------------------------------------------------------------------
        // КЛЮЧЕВОЕ:
        // 1) сохраняем в EEPROM
        // 2) применяем в TimeService
        // --------------------------------------------------------------------
        prefs.setTimezone((int32_t)tz.gmtOffset, (int32_t)tz.dstOffset);
        prefs.save();

        _time.setTimezone(tz.gmtOffset, tz.dstOffset);
    }

    _level = Level::ROOT;
    _tzEditing = false;
    _dirty = true;
}

void SettingsScreen::tzLeft()  { _dirty = true; }
void SettingsScreen::tzRight() { _dirty = true; }

void SettingsScreen::tzUp() {
    if (!_tzEditing) return;
    int n = sizeof(TZ_LIST) / sizeof(TZ_LIST[0]);
    _tzIndex = (_tzIndex + 1) % n;
}

void SettingsScreen::tzDown() {
    if (!_tzEditing) return;
    int n = sizeof(TZ_LIST) / sizeof(TZ_LIST[0]);
    _tzIndex = (_tzIndex + n - 1) % n;
}

void SettingsScreen::tzEnter() {
    // toggle edit
    _tzEditing = !_tzEditing;
    _dirty = true;
}

void SettingsScreen::tzExit() {
    // если в edit — просто выходим из edit
    // иначе — выходим из меню с apply=true (сохранить + применить)
    if (_tzEditing) {
        _tzEditing = false;
        _dirty = true;
    } else {
        exitTimezoneMenu(true);
    }
}

void SettingsScreen::drawTimezoneMenu() {
    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(20, 8);
    _tft.print("Timezone");

    _tft.setTextSize(1);
    _tft.setCursor(20, 48);

    uint16_t color = _tzEditing ? ST77XX_WHITE : th.accent;
    _tft.setTextColor(color, th.bg);
    _tft.print(TZ_LIST[_tzIndex].name);

    if (_tzEditing) {
        _tft.drawFastHLine(20, 56, 80, ST77XX_WHITE);
    }
}