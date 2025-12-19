#include "ui/BottomBar.h"

BottomBar::BottomBar(
    Adafruit_ST7735& tft,
    ThemeService& theme,
    LayoutService& layout,
    DhtService& dht
)
: _tft(tft)
, _theme(theme)
, _layout(layout)
, _dht(dht)
{}

void BottomBar::markDirty() {
    _dirty = true;
}

void BottomBar::draw() {
    if (!_dirty) return;
    _dirty = false;

    const Theme& th = _theme.current();

    const int y = _layout.bottomY();
    const int h = _layout.bottomH();

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextWrap(false);

    // фон
    _tft.fillRect(0, y, _tft.width(), h, th.bg);

    if (!_dht.isValid()) {
        _tft.setTextColor(th.muted, th.bg);
        _tft.setCursor(10, y + 10);
        _tft.print("Sensor...");
        return;
    }

    float t = _dht.temperature();
    float hmd = _dht.humidity();

    // 🌡 температура
    _tft.setTextColor(th.accent, th.bg);
    _tft.setCursor(10, y + 10);
    _tft.print("T");

    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(22, y + 10);
    _tft.printf("%.1fC", t);

    // 💧 влажность
    _tft.setTextColor(th.accent, th.bg);
    _tft.setCursor(90, y + 10);
    _tft.print("H");

    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(102, y + 10);
    _tft.printf("%.0f%%", hmd);
}