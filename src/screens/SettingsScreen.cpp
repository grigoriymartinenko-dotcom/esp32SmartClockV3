#include "screens/SettingsScreen.h"

// ============================================================================
// PreferencesService — глобальный объект, созданный в main.cpp
// Мы используем его тут только для сохранения настроек.
// ============================================================================
extern PreferencesService prefs;

// ============================================================================
// ВАЖНО ДЛЯ C++:
// static constexpr массивы, объявленные в .h, должны иметь ОДНО определение в .cpp
// (иначе возможны проблемы линковки в некоторых режимах компиляции).
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
// begin(): вызывается при входе на экран Settings
// ============================================================================
void SettingsScreen::begin() {
    _exitRequested = false;

    _level = Level::ROOT;
    _mode  = UiMode::NAV;

    // Стартовое выделение оставляем как есть (не сбрасываем _selected),
    // чтобы UX был приятнее: вернулся в Settings — курсор там же.
    _dirty = true;

    // Визуальная панель кнопок (не GPIO)
    _bar.setVisible(true);
    _bar.setActions(true, true, true, true);
    _bar.markDirty();
}

// ============================================================================
// update(): вызывается каждый loop() когда экран активен
// ============================================================================
void SettingsScreen::update() {
    // Если что-то изменилось — перерисуем
    if (_dirty) {
        redrawAll();
        _dirty = false;
    }

    // Bottom визуальная панель
    _bar.update();
}

// ============================================================================
// Смена темы -> перерисовать
// ============================================================================
void SettingsScreen::onThemeChanged() {
    _bar.markDirty();
    _dirty = true;
}

// ============================================================================
// ============================ BUTTONS (UX) ==================================
// ============================================================================
//
// Контракт (как мы зафиксировали):
//
// ROOT:
//   SHORT LEFT/RIGHT -> выбор пункта
//   LONG  OK         -> вход в подменю
//   LONG  BACK       -> выход из Settings (exitRequested)
//
// SUBMENU (Night / Timezone):
//   NAV:
//     SHORT LEFT/RIGHT -> выбор поля (Night) / (Timezone: поле одно -> ничего)
//     LONG  OK         -> enter EDIT
//     LONG  BACK       -> exit submenu (APPLY)
//
//   EDIT:
//     Night Start/End:
//       SHORT LEFT/RIGHT -> HH / MM
//       SHORT OK         -> + (inc)
//       SHORT BACK       -> - (dec)
//       LONG  OK         -> exit EDIT (APPLY edit)
//       LONG  BACK       -> cancel EDIT (ROLLBACK)
//     Night Mode:
//       SHORT OK/BACK    -> циклим AUTO/ON/OFF (+/-)
//       SHORT LEFT/RIGHT -> ничего (HH/MM не нужно)
//     Timezone:
//       SHORT OK         -> следующий
//       SHORT BACK       -> предыдущий
//       SHORT LEFT/RIGHT -> ничего (HH/MM нет)
//       LONG  OK         -> exit EDIT (APPLY edit)
//       LONG  BACK       -> cancel EDIT (ROLLBACK)
// ----------------------------------------------------------------------------

void SettingsScreen::onShortLeft() {
    if (_mode == UiMode::NAV) {
        navLeft();
        return;
    }

    // EDIT
    if (_level == Level::NIGHT) {
        // В EDIT для времени: LEFT выбирает HH (только если редактируем Start/End)
        if (_nightField == NightField::START || _nightField == NightField::END) {
            _timePart = TimePart::HH;
            _dirty = true;
        }
        return;
    }

    // TIMEZONE: в EDIT LEFT не нужен (там нет HH/MM)
}

void SettingsScreen::onShortRight() {
    if (_mode == UiMode::NAV) {
        navRight();
        return;
    }

    // EDIT
    if (_level == Level::NIGHT) {
        // В EDIT для времени: RIGHT выбирает MM (только если редактируем Start/End)
        if (_nightField == NightField::START || _nightField == NightField::END) {
            _timePart = TimePart::MM;
            _dirty = true;
        }
        return;
    }

    // TIMEZONE: в EDIT RIGHT не нужен
}

void SettingsScreen::onShortOk() {
    if (_mode != UiMode::EDIT) return;
    editInc(); // + (или "следующий" для timezone)
}

void SettingsScreen::onShortBack() {
    if (_mode != UiMode::EDIT) return;
    editDec(); // - (или "предыдущий" для timezone)
}

void SettingsScreen::onLongOk() {
    // ROOT -> вход в пункт меню
    if (_level == Level::ROOT) {
        enterSubmenu(MENU[_selected].target);
        return;
    }

    // SUBMENU:
    // NAV -> enter EDIT
    // EDIT -> apply edit и выйти в NAV
    if (_mode == UiMode::NAV) enterEdit();
    else                      exitEdit(true);
}

void SettingsScreen::onLongBack() {
    // ROOT -> выйти из Settings
    if (_level == Level::ROOT) {
        _exitRequested = true;
        return;
    }

    // SUBMENU:
    // EDIT -> cancel edit
    // NAV  -> выйти из submenu (APPLY settings)
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
// ============================ NAV / EDIT CORE ===============================
// ============================================================================

void SettingsScreen::navLeft() {
    if (_level == Level::ROOT) {
        const int n = sizeof(MENU) / sizeof(MENU[0]);
        _selected = (_selected + n - 1) % n;
        _dirty = true;
        return;
    }

    if (_level == Level::NIGHT) {
        // LEFT/RIGHT в NAV выбирают поле MODE/START/END
        _nightField = (_nightField == NightField::MODE)
            ? NightField::END
            : (NightField)((int)_nightField - 1);
        _dirty = true;
        return;
    }

    // TIMEZONE: в NAV поле одно — нечего выбирать (UX предсказуемый)
}

void SettingsScreen::navRight() {
    if (_level == Level::ROOT) {
        const int n = sizeof(MENU) / sizeof(MENU[0]);
        _selected = (_selected + 1) % n;
        _dirty = true;
        return;
    }

    if (_level == Level::NIGHT) {
        _nightField = (_nightField == NightField::END)
            ? NightField::MODE
            : (NightField)((int)_nightField + 1);
        _dirty = true;
        return;
    }

    // TIMEZONE: в NAV поле одно — ничего не делаем
}

void SettingsScreen::enterSubmenu(Level lvl) {
    // Плейсхолдеры пока не открываем
    if (lvl == Level::ROOT) return;

    _level = lvl;
    _mode  = UiMode::NAV;

    if (lvl == Level::NIGHT) {
        // Берём текущие значения как старт
        _tmpMode = _night.mode();
        _tmpStartMin = _night.autoStart();
        _tmpEndMin   = _night.autoEnd();

        // И сохраняем бэкап для CANCEL EDIT
        _bakMode = _tmpMode;
        _bakStartMin = _tmpStartMin;
        _bakEndMin   = _tmpEndMin;

        _nightField = NightField::MODE;
        _timePart = TimePart::HH;
    }

    if (lvl == Level::TIMEZONE) {
        // При входе выставим tzIndex по сохранённым prefs,
        // чтобы пользователь видел актуальный выбор.
        long savedGmt = (long)prefs.tzGmtOffset();
        int  savedDst = (int)prefs.tzDstOffset();

        int n = sizeof(TZ_LIST) / sizeof(TZ_LIST[0]);
        int found = 0;
        for (int i = 0; i < n; i++) {
            if (TZ_LIST[i].gmtOffset == savedGmt && TZ_LIST[i].dstOffset == savedDst) {
                _tzIndex = i;
                found = 1;
                break;
            }
        }
        if (!found) _tzIndex = 0;

        // Бэкап для CANCEL EDIT
        _bakTzIndex = _tzIndex;
    }

    _dirty = true;
}

void SettingsScreen::exitSubmenu(bool apply) {
    // apply=true — применяем и сохраняем то, что сейчас в _tmp / _tzIndex

    if (_level == Level::NIGHT && apply) {
        // Применяем в сервис
        _night.setMode(_tmpMode);
        _night.setAutoRange(_tmpStartMin, _tmpEndMin);

        // Сохраняем в EEPROM
        prefs.setNightMode(
            _tmpMode == NightService::Mode::AUTO ? NightModePref::AUTO :
            _tmpMode == NightService::Mode::ON   ? NightModePref::ON :
                                                   NightModePref::OFF
        );
        prefs.setNightRange((uint16_t)_tmpStartMin, (uint16_t)_tmpEndMin);
        prefs.save();
    }

    if (_level == Level::TIMEZONE && apply) {
        const TzItem& tz = TZ_LIST[_tzIndex];

        // Сохраняем и применяем
        prefs.setTimezone((int32_t)tz.gmtOffset, (int32_t)tz.dstOffset);
        prefs.save();

        _time.setTimezone(tz.gmtOffset, tz.dstOffset);
    }

    // Возврат в ROOT
    _level = Level::ROOT;
    _mode  = UiMode::NAV;
    _dirty = true;
}

void SettingsScreen::enterEdit() {
    _mode = UiMode::EDIT;

    // В Night, если вошли в EDIT на Start/End — начнём с HH
    if (_level == Level::NIGHT) {
        if (_nightField == NightField::START || _nightField == NightField::END) {
            _timePart = TimePart::HH;
        }
    }

    // Для Timezone — просто EDIT режим
    _dirty = true;
}

void SettingsScreen::exitEdit(bool apply) {
    // apply=false -> rollback только "внутреннего редактирования",
    // не выход из submenu. (выход из submenu — отдельная кнопка LONG BACK в NAV)

    if (!apply) {
        if (_level == Level::NIGHT) {
            _tmpMode     = _bakMode;
            _tmpStartMin = _bakStartMin;
            _tmpEndMin   = _bakEndMin;
        }
        if (_level == Level::TIMEZONE) {
            _tzIndex = _bakTzIndex;
        }
    } else {
        // apply=true: фиксируем текущие значения как "новая база" для возможного CANCEL дальше
        if (_level == Level::NIGHT) {
            _bakMode     = _tmpMode;
            _bakStartMin = _tmpStartMin;
            _bakEndMin   = _tmpEndMin;
        }
        if (_level == Level::TIMEZONE) {
            _bakTzIndex = _tzIndex;
        }
    }

    _mode = UiMode::NAV;
    _dirty = true;
}

void SettingsScreen::editInc() {
    // NIGHT
    if (_level == Level::NIGHT) {
        if (_nightField == NightField::MODE) {
            // + циклим режим
            _tmpMode = (NightService::Mode)(((int)_tmpMode + 1) % 3);
            _dirty = true;
            return;
        }

        // START/END: + увеличиваем HH или MM
        int* v = (_nightField == NightField::START) ? &_tmpStartMin : &_tmpEndMin;
        int hh = (*v) / 60;
        int mm = (*v) % 60;

        if (_timePart == TimePart::HH) hh = (hh + 1) % 24;
        else                           mm = (mm + 1) % 60;

        *v = hh * 60 + mm;
        _dirty = true;
        return;
    }

    // TIMEZONE
    if (_level == Level::TIMEZONE) {
        int n = sizeof(TZ_LIST) / sizeof(TZ_LIST[0]);
        _tzIndex = (_tzIndex + 1) % n;
        _dirty = true;
        return;
    }
}

void SettingsScreen::editDec() {
    // NIGHT
    if (_level == Level::NIGHT) {
        if (_nightField == NightField::MODE) {
            // - циклим режим назад
            int m = (int)_tmpMode - 1;
            if (m < 0) m = 2;
            _tmpMode = (NightService::Mode)m;
            _dirty = true;
            return;
        }

        // START/END: - уменьшаем HH или MM
        int* v = (_nightField == NightField::START) ? &_tmpStartMin : &_tmpEndMin;
        int hh = (*v) / 60;
        int mm = (*v) % 60;

        if (_timePart == TimePart::HH) hh = (hh + 23) % 24;
        else                           mm = (mm + 59) % 60;

        *v = hh * 60 + mm;
        _dirty = true;
        return;
    }

    // TIMEZONE
    if (_level == Level::TIMEZONE) {
        int n = sizeof(TZ_LIST) / sizeof(TZ_LIST[0]);
        _tzIndex = (_tzIndex + n - 1) % n;
        _dirty = true;
        return;
    }
}

// ============================================================================
// =============================== DRAW =======================================
// ============================================================================

void SettingsScreen::redrawAll() {
    const Theme& th = theme();

    _tft.fillScreen(th.bg);

    if (_level == Level::ROOT) {
        drawRoot();
    } else if (_level == Level::NIGHT) {
        drawNight();
    } else {
        drawTimezone();
    }

    // Принудительно перерисуем bar (чтобы тема/режим не оставляли мусор)
    _bar.markDirty();
}

void SettingsScreen::drawRoot() {
    const Theme& th = theme();

    // ---- Title
    _tft.setTextSize(2);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(20, 8);
    _tft.print("SETTINGS");

    // ---- List
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
    const Theme& th = theme();

    // ---- Title
    _tft.setTextSize(2);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(12, 8);
    _tft.print("Night Mode");

    _tft.setTextSize(1);

    // Визуальная подсветка:
    // - выбранное поле в NAV: accent
    // - выбранное поле в EDIT: white
    auto rowColor = [&](NightField f) -> uint16_t {
        if (_nightField != f) return th.textPrimary;
        return (_mode == UiMode::EDIT) ? ST77XX_WHITE : th.accent;
    };

    const int y0 = 40;
    const int row = 18;

    // Row 1: MODE
    _tft.setCursor(12, y0);
    _tft.setTextColor(rowColor(NightField::MODE), th.bg);
    _tft.print("Mode: ");
    _tft.print(
        _tmpMode == NightService::Mode::AUTO ? "AUTO" :
        _tmpMode == NightService::Mode::ON   ? "ON"   : "OFF"
    );

    // Row 2: START
    _tft.setCursor(12, y0 + row);
    _tft.setTextColor(rowColor(NightField::START), th.bg);
    _tft.printf("Start: %02d:%02d", _tmpStartMin / 60, _tmpStartMin % 60);

    // Row 3: END
    _tft.setCursor(12, y0 + 2 * row);
    _tft.setTextColor(rowColor(NightField::END), th.bg);
    _tft.printf("End:   %02d:%02d", _tmpEndMin / 60, _tmpEndMin % 60);

    // Доп. подсказка для EDIT времени: подчёркиваем HH/MM
    // Это помогает "не потеряться".
    if (_mode == UiMode::EDIT && (_nightField == NightField::START || _nightField == NightField::END)) {

        // Позиции "HH:MM" примерно:
        // "Start: " = 7 символов * 6px = 42px (примерно)
        // лучше сделать чуть проще: фиксированный x после текста.
        // Здесь мы не используем GFX измерение текста, чтобы не усложнять.
        const int baseX = 12 + 42;
        const int underlineYStart = y0 + ((_nightField == NightField::START) ? row : 2 * row) + 10;

        if (_timePart == TimePart::HH) {
            _tft.drawFastHLine(baseX, underlineYStart + 8, 12, ST77XX_WHITE); // HH
        } else {
            _tft.drawFastHLine(baseX + 18, underlineYStart + 8, 12, ST77XX_WHITE); // MM
        }
    }
}

void SettingsScreen::drawTimezone() {
    const Theme& th = theme();

    // ---- Title
    _tft.setTextSize(2);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(18, 8);
    _tft.print("Timezone");

    _tft.setTextSize(1);

    // Одно поле:
    // NAV: accent, EDIT: white
    uint16_t color = (_mode == UiMode::EDIT) ? ST77XX_WHITE : th.accent;

    _tft.setCursor(12, 44);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.print("Zone:");

    _tft.setCursor(12, 60);
    _tft.setTextColor(color, th.bg);
    _tft.print(TZ_LIST[_tzIndex].name);

    // Подчёркивание в EDIT как явный индикатор режима
    if (_mode == UiMode::EDIT) {
        _tft.drawFastHLine(12, 68, 80, ST77XX_WHITE);
    }
}