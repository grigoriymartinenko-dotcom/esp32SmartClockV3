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

    // важно: помним старое место для очистки
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

    // Если раньше рисовали — и теперь нужно стереть старое место (смена видимости или смена y)
    const bool needClearOld =
        (_wasVisible && (_dirty || !_visible || _y < 0) && _lastY >= 0);

    if (needClearOld) {
        clearAt(_lastY);
    }

    // Скрыт → ничего не рисуем
    if (!_visible) {
        _wasVisible = false;
        _dirty = false;
        return;
    }

    // Невалидный Y → ничего не рисуем
    if (_y < 0) {
        _wasVisible = false;
        _dirty = false;
        return;
    }

    // Если не dirty и уже было нарисовано — можно выйти
    if (!_dirty && _wasVisible && _lastY == _y)
        return;

    // Рисуем линию
    draw();

    _lastY = _y;
    _wasVisible = true;
    _dirty = false;
}

void UiSeparator::draw() {
    const Theme& th = _theme.current();

    _tft.drawFastHLine(
        0,
        _y,
        _tft.width(),
        th.muted
    );
}

void UiSeparator::clearAt(int y) {
    if (y < 0) return;

    const Theme& th = _theme.current();

    // очищаем область толщиной 2px на всякий случай
    _tft.fillRect(
        0,
        y,
        _tft.width(),
        2,
        th.bg
    );
}