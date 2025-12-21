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
    , _bar(tft, themeService, layoutService)
    , _night(nightService)
    , _ui(uiVersion)
{
}

void SettingsScreen::begin() {
    _exitRequested = false;

    _bar.setVisible(true);
    _bar.setActions(true, true, true, true);

    // Settings использует flash(), не highlight
    _bar.setHighlight(false, false, false, false);

    _dirty = true;
    redrawAll();
}

void SettingsScreen::update() {

    // защитный сброс highlight (если "протёк" из другого экрана)
    _bar.setHighlight(false, false, false, false);

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

// ---------------- short press ----------------

void SettingsScreen::onLeft() {
    _bar.flash(ButtonBar::ButtonId::LEFT);

    if (_selected > 0) {
        _selected--;
        _dirty = true;
    }
}

void SettingsScreen::onRight() {
    _bar.flash(ButtonBar::ButtonId::RIGHT);

    if (_selected < ITEM_COUNT - 1) {
        _selected++;
        _dirty = true;
    }
}

void SettingsScreen::onOk() {
    _bar.flash(ButtonBar::ButtonId::OK);

    // пока базовые действия:
    // 0: Wi-Fi (заглушка)
    // 1: Timezone (будет подменю)
    // 2: Night mode (будет подменю)
    // 3: Future 1
    // 4: Future 2
    // 5: About

    // Ничего не делаем здесь для сложных пунктов — они уйдут в подменю
}

void SettingsScreen::onBack() {
    _bar.flash(ButtonBar::ButtonId::BACK);
    // короткий BACK — ничего (долгий будет "назад/выход")
}

// ---------------- long press ----------------

void SettingsScreen::onOkLong() {
    _bar.flash(ButtonBar::ButtonId::OK);

    // LONG OK = "войти" в выбранный пункт (подменю)
    // Сейчас — заготовка: просто пометим экран грязным
    // Следующий шаг: переключение internal-level (ROOT -> SUBMENU)
    _dirty = true;
}

void SettingsScreen::onBackLong() {
    _bar.flash(ButtonBar::ButtonId::BACK);

    // LONG BACK = выйти из Settings (или выйти из подменю назад)
    // Сейчас подменю ещё нет -> сразу выход
    _exitRequested = true;
}

// ---------------- exit flag ----------------

bool SettingsScreen::exitRequested() const {
    return _exitRequested;
}

void SettingsScreen::clearExitRequest() {
    _exitRequested = false;
}

// ---------------- draw ----------------

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

void SettingsScreen::drawList() {
    const Theme& th = theme();

    const int contentTop = 36;
    const int contentBottom = _layout.buttonBarY();
    const int contentH = contentBottom - contentTop;

    const int rowH = contentH / ITEM_COUNT;
    const int xPad = 12;

    for (int i = 0; i < ITEM_COUNT; i++) {
        const int y = contentTop + i * rowH;

        _tft.fillRect(0, y, _tft.width(), rowH, th.bg);

        bool sel = (i == _selected);
        uint16_t color = sel ? th.accent : th.textPrimary;

        _tft.setTextSize(1);
        _tft.setTextColor(color, th.bg);
        _tft.setCursor(xPad, y + (rowH - 8) / 2);

        if (i == 0) _tft.print("Wi-Fi");
        if (i == 1) _tft.print("Timezone");
        if (i == 2) _tft.print("Night mode");
        if (i == 3) _tft.print("Future 1");
        if (i == 4) _tft.print("Future 2");
        if (i == 5) _tft.print("About");

        // индикатор ">"
        _tft.setTextColor(sel ? th.accent : th.muted, th.bg);
        _tft.setCursor(_tft.width() - 12, y + (rowH - 8) / 2);
        _tft.print(">");
    }
}