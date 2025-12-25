#include "ui/UiSeparator.h"

UiSeparator::UiSeparator(
    Adafruit_ST7735& tft,
    ThemeService& themeService,
    LayoutService& layoutService
)
    : _tft(tft)
    , _theme(themeService)
    , _layout(layoutService)
{
}

void UiSeparator::setY(int y) {
    if (_y == y)
        return;

    _y = y;
    _dirty = true;
}

void UiSeparator::setVisible(bool visible) {
    if (_visible == visible)
        return;

    _visible = visible;
    _dirty = true;
}

void UiSeparator::markDirty() {
    _dirty = true;
}

void UiSeparator::update() {

    // --- если скрыт ---
    if (!_visible) {
        if (_wasVisible) {
            clear();
            _wasVisible = false;
        }
        return;
    }

    // --- если Y невалиден ---
    if (_y < 0) {
        if (_wasVisible) {
            clear();
            _wasVisible = false;
        }
        return;
    }

    if (!_dirty && _wasVisible)
        return;

    draw();
    _dirty = false;
    _wasVisible = true;
}

void UiSeparator::draw() {
    const Theme& th = _theme.current();

    // толщина линии = 1px
    _tft.drawFastHLine(
        0,
        _y,
        _tft.width(),
        th.muted
    );
}

void UiSeparator::clear() {
    const Theme& th = _theme.current();

    // очищаем область толщиной 2px на всякий случай
    _tft.fillRect(
        0,
        _y,
        _tft.width(),
        2,
        th.bg
    );
}