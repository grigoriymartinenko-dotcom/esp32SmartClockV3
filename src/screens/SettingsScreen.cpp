#include "screens/SettingsScreen.h"

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
    , _night(nightService)
    , _uiVersion(uiVersion)
    , _bar(tft, themeService, layoutService)
{
}

void SettingsScreen::begin() {
    _exitRequested = false;

    _bar.setVisible(true);
    _bar.setActions(true, true, true, true);

    // SettingsScreen использует только flash()
    _bar.setHighlight(false, false, false, false);

    // подтягиваем актуальные значения из NightService
    _tmpStartMin = _night.autoStart();
    _tmpEndMin   = _night.autoEnd();

    _lastMode = _night.mode();
    _lastStartMin = _tmpStartMin;
    _lastEndMin   = _tmpEndMin;

    _editState = EditState::NONE;
    _editField = EditField::HOURS;

    _dirty = true;
    redrawAll();
}

void SettingsScreen::update() {

    // 🔒 защитный сброс highlight
    _bar.setHighlight(false, false, false, false);

    // мигание HH/MM использует TIME-версию (тик уже приходит из TimeService)
    uint32_t blinkV = _uiVersion.version(UiChannel::TIME);

    bool need = false;

    // если изменился тик мигания и мы в режиме редактирования — перерисуем список
    if (_editState != EditState::NONE && blinkV != _lastBlinkV) {
        _lastBlinkV = blinkV;
        need = true;
    }

    // если вдруг NightService изменился извне (на будущее) — тоже перерисуем
    if (_night.mode() != _lastMode) {
        _lastMode = _night.mode();
        need = true;
    }
    if (_night.autoStart() != _lastStartMin) {
        _lastStartMin = _night.autoStart();
        need = true;
    }
    if (_night.autoEnd() != _lastEndMin) {
        _lastEndMin = _night.autoEnd();
        need = true;
    }

    if (_dirty || need) {
        redrawAll();
        _dirty = false;
    }

    _bar.update();
}

void SettingsScreen::onThemeChanged() {
    _bar.markDirty();
    _dirty = true;
}

bool SettingsScreen::exitRequested() const {
    return _exitRequested;
}

void SettingsScreen::clearExitRequest() {
    _exitRequested = false;
}

// =====================================================
// Input
// =====================================================

void SettingsScreen::onLeft() {
    _bar.flash(ButtonBar::ButtonId::LEFT);

    if (_editState != EditState::NONE) {
        applyEditDelta(-1);
        _dirty = true;
        return;
    }

    if (_selected > 0) {
        _selected--;
        _dirty = true;
    }
}

void SettingsScreen::onRight() {
    _bar.flash(ButtonBar::ButtonId::RIGHT);

    if (_editState != EditState::NONE) {
        applyEditDelta(+1);
        _dirty = true;
        return;
    }

    if (_selected < (int)ITEM_COUNT - 1) {
        _selected++;
        _dirty = true;
    }
}

void SettingsScreen::onOk() {
    _bar.flash(ButtonBar::ButtonId::OK);

    if (_editState != EditState::NONE) {
        // в edit OK переключает HH/MM
        toggleEditField();
        _dirty = true;
        return;
    }

    // обычный режим: действия по пунктам
    if (_selected == ITEM_NIGHT_MODE) {
        cycleNightMode();
        _dirty = true;
        return;
    }

    if (_selected == ITEM_NIGHT_START) {
        if (isAuto()) {
            startEdit(EditState::EDIT_START);
            _dirty = true;
        }
        return;
    }

    if (_selected == ITEM_NIGHT_END) {
        if (isAuto()) {
            startEdit(EditState::EDIT_END);
            _dirty = true;
        }
        return;
    }

    // About — заглушка (пока ничего)
}

void SettingsScreen::onBack() {
    _bar.flash(ButtonBar::ButtonId::BACK);

    if (_editState != EditState::NONE) {
        // BACK в edit = сохранить и выйти
        commitEditAndExit();
        _dirty = true;
        return;
    }

    // обычный BACK = выход
    _exitRequested = true;
}

// =====================================================
// Night mode actions
// =====================================================

bool SettingsScreen::isAuto() const {
    return _night.mode() == NightService::Mode::AUTO;
}

void SettingsScreen::cycleNightMode() {
    NightService::Mode m = _night.mode();

    if (m == NightService::Mode::AUTO) m = NightService::Mode::ON;
    else if (m == NightService::Mode::ON) m = NightService::Mode::OFF;
    else m = NightService::Mode::AUTO;

    _night.setMode(m);

    // если вернулись в AUTO — подтянем range из сервиса
    _tmpStartMin = _night.autoStart();
    _tmpEndMin   = _night.autoEnd();

    _lastMode = _night.mode();
    _lastStartMin = _night.autoStart();
    _lastEndMin   = _night.autoEnd();
}

void SettingsScreen::startEdit(EditState s) {
    _editState = s;
    _editField = EditField::HOURS;

    // синхронизация с сервисом на вход в edit
    _tmpStartMin = _night.autoStart();
    _tmpEndMin   = _night.autoEnd();

    _lastBlinkV = _uiVersion.version(UiChannel::TIME);
}

void SettingsScreen::toggleEditField() {
    _editField = (_editField == EditField::HOURS)
        ? EditField::MINUTES
        : EditField::HOURS;
}

int SettingsScreen::clampMin(int v) {
    if (v < 0) return 0;
    if (v > 1439) return 1439;
    return v;
}

int SettingsScreen::addMinutes(int v, int delta) {
    // циклически по суткам
    int r = v + delta;
    while (r < 0) r += 1440;
    while (r >= 1440) r -= 1440;
    return r;
}

void SettingsScreen::applyEditDelta(int delta) {

    int* target = nullptr;
    if (_editState == EditState::EDIT_START) target = &_tmpStartMin;
    if (_editState == EditState::EDIT_END)   target = &_tmpEndMin;
    if (!target) return;

    int v = *target;
    int hh = v / 60;
    int mm = v % 60;

    if (_editField == EditField::HOURS) {
        hh = (hh + delta) % 24;
        if (hh < 0) hh += 24;
    } else {
        mm = (mm + delta) % 60;
        if (mm < 0) mm += 60;
    }

    *target = hh * 60 + mm;
}

void SettingsScreen::commitEditAndExit() {

    // коммитим только в AUTO
    if (isAuto()) {
        _night.setAutoRange(_tmpStartMin, _tmpEndMin);

        // обновим кеш
        _lastStartMin = _night.autoStart();
        _lastEndMin   = _night.autoEnd();
    }

    _editState = EditState::NONE;
    _editField = EditField::HOURS;
}

// =====================================================
// Draw
// =====================================================

void SettingsScreen::redrawAll() {
    const Theme& th = theme();

    _tft.fillScreen(th.bg);

    drawTitle();
    drawList();

    _bar.markDirty();
}

void SettingsScreen::drawTitle() {
    const Theme& th = theme();

    _tft.setFont(nullptr);
    _tft.setTextWrap(false);
    _tft.setTextSize(2);
    _tft.setTextColor(th.textPrimary, th.bg);

    const int y = 8;
    const char* title = "SETTINGS";

    int len = 0;
    for (const char* p = title; *p; ++p) len++;
    int textW = len * 12;

    int x = (_tft.width() - textW) / 2;
    _tft.setCursor(x, y);
    _tft.print(title);

    _tft.fillRect(0, y + 22, _tft.width(), 6, th.bg);
}

void SettingsScreen::formatHHMM(char* out, int minutes) {
    int hh = (minutes / 60) % 24;
    int mm = minutes % 60;
    out[0] = '0' + (hh / 10);
    out[1] = '0' + (hh % 10);
    out[2] = ':';
    out[3] = '0' + (mm / 10);
    out[4] = '0' + (mm % 10);
    out[5] = '\0';
}

void SettingsScreen::drawList() {
    const Theme& th = theme();

    const int contentTop = 36;
    const int contentBottom = _layout.buttonBarY();
    const int contentH = contentBottom - contentTop;

    const int rowH = contentH / (int)ITEM_COUNT;
    const int xPad = 12;

    const bool autoMode = isAuto();

    // blink для редактирования (HH/MM)
    const bool blinkOn = (_uiVersion.version(UiChannel::TIME) % 2) == 0;

    for (int i = 0; i < (int)ITEM_COUNT; i++) {
        const int y = contentTop + i * rowH;
        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        const bool sel = (i == _selected);

        // базовый цвет строки
        uint16_t leftColor = sel ? th.accent : th.textPrimary;

        // Start/End в ON/OFF делаем muted (и даже если selected — тоже muted, чтобы было явно "недоступно")
        if ((i == ITEM_NIGHT_START || i == ITEM_NIGHT_END) && !autoMode) {
            leftColor = th.muted;
        }

        _tft.setFont(nullptr);
        _tft.setTextWrap(false);
        _tft.setTextSize(1);
        _tft.setTextColor(leftColor, th.bg);
        _tft.setCursor(xPad, y + (rowH - 8) / 2);

        if (i == ITEM_NIGHT_MODE)  _tft.print("Night mode");
        if (i == ITEM_NIGHT_START) _tft.print("Night start");
        if (i == ITEM_NIGHT_END)   _tft.print("Night end");
        if (i == ITEM_ABOUT)       _tft.print("About");

        // ===== Right value (mode / time) =====
        char val[8] = {0};

        if (i == ITEM_NIGHT_MODE) {
            const char* m = "AUTO";
            if (_night.mode() == NightService::Mode::ON)  m = "ON";
            if (_night.mode() == NightService::Mode::OFF) m = "OFF";

            // печатаем справа строкой
            int vlen = 0; for (const char* p = m; *p; ++p) vlen++;
            int textW = vlen * 6;

            uint16_t rc = sel ? th.accent : th.muted;
            _tft.setTextColor(rc, th.bg);
            _tft.setCursor(_tft.width() - xPad - textW, y + (rowH - 8) / 2);
            _tft.print(m);
        }

        if (i == ITEM_NIGHT_START || i == ITEM_NIGHT_END) {
            int minutes = (i == ITEM_NIGHT_START) ? _night.autoStart() : _night.autoEnd();

            // если мы в редактировании — показываем tmp
            if (_editState == EditState::EDIT_START && i == ITEM_NIGHT_START) minutes = _tmpStartMin;
            if (_editState == EditState::EDIT_END   && i == ITEM_NIGHT_END)   minutes = _tmpEndMin;

            formatHHMM(val, minutes);

            // в ON/OFF отображаем всё равно, но muted
            uint16_t rc = sel ? th.accent : th.muted;
            if (!autoMode) rc = th.muted;

            // рисуем справа "HH:MM", с миганием выбранного поля в edit
            // Для мигания: вместо цифр ставим пробелы, когда blinkOff.
            if (autoMode) {
                const bool editingThis =
                    (_editState == EditState::EDIT_START && i == ITEM_NIGHT_START) ||
                    (_editState == EditState::EDIT_END   && i == ITEM_NIGHT_END);

                if (editingThis && !blinkOn) {
                    if (_editField == EditField::HOURS) {
                        val[0] = ' ';
                        val[1] = ' ';
                    } else {
                        val[3] = ' ';
                        val[4] = ' ';
                    }
                }
            }

            int vlen = 5; // "HH:MM"
            int textW = vlen * 6;

            _tft.setTextColor(rc, th.bg);
            _tft.setCursor(_tft.width() - xPad - textW, y + (rowH - 8) / 2);
            _tft.print(val);
        }

        // “>” индикатор (в Start/End в ON/OFF рисуем muted)
        uint16_t arrowC = sel ? th.accent : th.muted;
        if ((i == ITEM_NIGHT_START || i == ITEM_NIGHT_END) && !autoMode) {
            arrowC = th.muted;
        }

        _tft.setTextColor(arrowC, th.bg);
        _tft.setCursor(_tft.width() - 12, y + (rowH - 8) / 2);
        _tft.print(">");
    }
}