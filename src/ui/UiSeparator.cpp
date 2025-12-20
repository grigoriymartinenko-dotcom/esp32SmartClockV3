#include "ui/UiSeparator.h"

/*
 * UiSeparator
 * -----------
 * ФИЗИЧЕСКАЯ отрисовка линии.
 * Толщина линии = 2px (ST7735 safe).
 */

UiSeparator::UiSeparator(
    Adafruit_ST7735& tft,
    ThemeService& theme,
    int y
)
: _tft(tft)
, _theme(theme)
, _y(y)
{}

void UiSeparator::setY(int y) {
    if (_y != y) {
        _y = y;
        markDirty();
    }
}

void UiSeparator::markDirty() {
    _dirty = true;
}

void UiSeparator::update() {
    if (!_dirty) return;
    _dirty = false;
    draw();
}

void UiSeparator::draw() {

    if (_y < 0) return;

    const Theme& th = _theme.current();

    // 1px фон + 1px линия (ST7735 FIX)
    _tft.fillRect(
        0,
        _y,
        _tft.width(),
        2,
        th.bg
    );

    _tft.drawFastHLine(
        0,
        _y + 1,
        _tft.width(),
        th.accent
    );
}