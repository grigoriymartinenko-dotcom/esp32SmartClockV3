#include "screens/SettingsScreen.h"

SettingsScreen::SettingsScreen(
    Adafruit_ST7735& tft,
    ThemeService& themeService,
    LayoutService& layoutService
)
    : Screen(themeService)
    , _tft(tft)
    , _layout(layoutService)
    , _bar(tft, themeService, layoutService)
{
}

void SettingsScreen::begin() {
    _exitRequested = false;

    // ButtonBar доступен всегда
    _bar.setVisible(true);
    _bar.setActions(true, true, true, true);

    // ❌ НИКАКОЙ постоянной подсветки в Settings
    _bar.setHighlight(false, false, false, false);

    _dirty = true;
    redrawAll();
}

void SettingsScreen::update() {

    // ==================================================
    // 🔒 ЗАЩИТНЫЙ СБРОС highlight
    // ==================================================
    // SettingsScreen использует ТОЛЬКО flash().
    // Если highlight "протёк" из другого экрана —
    // он будет сброшен здесь гарантированно.
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

    // демо-действия
    // 0: Wi-Fi (пока заглушка)
    // 1: Timezone (пока заглушка)
    // 2: Night mode
    // 3: About (пока заглушка)
    if (_selected == 2) {
        _nightAuto = !_nightAuto;
        _dirty = true;
    }
}

void SettingsScreen::onBack() {
    _bar.flash(ButtonBar::ButtonId::BACK);
    _exitRequested = true;
}

bool SettingsScreen::exitRequested() const {
    return _exitRequested;
}

void SettingsScreen::clearExitRequest() {
    _exitRequested = false;
}

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
        if (i == 3) _tft.print("About");

        if (i == 2) {
            const char* val = _nightAuto ? "AUTO" : "MANUAL";

            int vlen = 0;
            for (const char* p = val; *p; ++p) vlen++;
            int textW = vlen * 6;

            _tft.setTextColor(sel ? th.accent : th.muted, th.bg);
            _tft.setCursor(_tft.width() - xPad - textW, y + (rowH - 8) / 2);
            _tft.print(val);
        }

        _tft.setTextColor(sel ? th.accent : th.muted, th.bg);
        _tft.setCursor(_tft.width() - 12, y + (rowH - 8) / 2);
        _tft.print(">");
    }
}