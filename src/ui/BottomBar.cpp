#include "ui/BottomBar.h"

BottomBar::BottomBar(
    Adafruit_ST7735& tft,
    ThemeService& themeService,
    LayoutService& layoutService,
    DhtService& dhtService
)
: _tft(tft),
  _themeService(themeService),
  _layout(layoutService),
  _dht(dhtService)
{}

void BottomBar::markDirty() {
    _dirty = true;
}

void BottomBar::setVisible(bool visible) {
    if (_visible != visible) {
        _visible = visible;
        _dirty = true;
    }
}

void BottomBar::draw() {
    if (!_visible) {
        if (_wasVisible) {
            clearInternal();      // стереть ОДИН раз
            _wasVisible = false;
        }
        return;
    }

    _wasVisible = true;

    if (!_dirty) return;

    clearInternal();
    drawContent();
    _dirty = false;
}

void BottomBar::clearInternal() {
    const Theme& theme = _themeService.current();

    _tft.fillRect(
        0,
        _layout.bottomY(),   // ✅ корректное имя
        _tft.width(),
        _layout.bottomH(),   // ✅ корректное имя
        theme.bg             // ✅ каноничный фон
    );
}

void BottomBar::drawContent() {
    const Theme& theme = _themeService.current();

    int temp = _dht.temperature();   // °C
    int hum  = _dht.humidity();      // %

    // ⚠️ ТВОЙ существующий код отрисовки
    // пример (если нужно):
    //
    // _tft.setTextColor(theme.textPrimary);
    // _tft.setCursor(4, _layout.bottomY() + 12);
    // _tft.printf("%dC  %d%%", temp, hum);
}