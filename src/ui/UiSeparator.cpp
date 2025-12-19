#include "ui/UiSeparator.h"

UiSeparator::UiSeparator(
    Adafruit_ST7735& tft,
    ThemeService& theme,
    int y
)
: _tft(tft), _theme(theme), _y(y)
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

void UiSeparator::draw() {
    if (!_dirty) return;
    _dirty = false;

    const Theme& th = _theme.current();

    _tft.drawFastHLine(
        0,
        _y,
        _tft.width(),
        th.accent
    );
}