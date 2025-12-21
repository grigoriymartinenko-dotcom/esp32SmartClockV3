#include "ui/ButtonBar.h"

ButtonBar::ButtonBar(
    Adafruit_ST7735& tft,
    ThemeService& themeService,
    LayoutService& layoutService
)
    : _tft(tft)
    , _themeService(themeService)
    , _layout(layoutService)
{
}

void ButtonBar::setVisible(bool visible) {
    if (_visible == visible) return;
    _visible = visible;
    _dirty = true;
}

void ButtonBar::setActions(bool left, bool ok, bool right, bool back) {
    if (_hasLeft == left && _hasOk == ok && _hasRight == right && _hasBack == back)
        return;

    _hasLeft = left;
    _hasOk = ok;
    _hasRight = right;
    _hasBack = back;
    _dirty = true;
}

void ButtonBar::setHighlight(bool left, bool ok, bool right, bool back) {
    if (_hiLeft == left && _hiOk == ok && _hiRight == right && _hiBack == back)
        return;

    _hiLeft = left;
    _hiOk = ok;
    _hiRight = right;
    _hiBack = back;
    _dirty = true;
}

// =====================================================
// FLASH — только если не активен
// =====================================================
void ButtonBar::flash(ButtonId id) {
    switch (id) {
        case ButtonId::LEFT:
            if (_flashLeft == 0) _flashLeft = FLASH_FRAMES;
            break;
        case ButtonId::OK:
            if (_flashOk == 0) _flashOk = FLASH_FRAMES;
            break;
        case ButtonId::RIGHT:
            if (_flashRight == 0) _flashRight = FLASH_FRAMES;
            break;
        case ButtonId::BACK:
            if (_flashBack == 0) _flashBack = FLASH_FRAMES;
            break;
    }
    _dirty = true;
}

bool ButtonBar::anyFlashActive() const {
    return (_flashLeft > 0) || (_flashOk > 0) || (_flashRight > 0) || (_flashBack > 0);
}

void ButtonBar::markDirty() {
    _dirty = true;
}

void ButtonBar::update() {
    if (!_visible) {
        if (_wasVisible) {
            clear();
            _wasVisible = false;
        }
        return;
    }

    if (anyFlashActive()) {
        _dirty = true;
    }

    if (_dirty || !_wasVisible) {
        draw();
        _dirty = false;
        _wasVisible = true;

        if (_flashLeft  > 0) _flashLeft--;
        if (_flashOk    > 0) _flashOk--;
        if (_flashRight > 0) _flashRight--;
        if (_flashBack  > 0) _flashBack--;
    }
}

void ButtonBar::clear() {
    const int y = _layout.buttonBarY();
    const int h = _layout.buttonBarH();
    _tft.fillRect(0, y, _tft.width(), h, _themeService.current().bg);
}

void ButtonBar::draw() {
    const Theme& th = _themeService.current();

    const int y = _layout.buttonBarY();
    const int h = _layout.buttonBarH();

    _tft.fillRect(0, y, _tft.width(), h, th.bg);

    const int w = _tft.width();
    const int cellW = w / 4;

    drawCell(0 * cellW, y, cellW, h, "<-",   _hasLeft,  _hiLeft,  _flashLeft  > 0);
    drawCell(1 * cellW, y, cellW, h, "OK",   _hasOk,    _hiOk,    _flashOk    > 0);
    drawCell(2 * cellW, y, cellW, h, "->",   _hasRight, _hiRight, _flashRight > 0);
    drawCell(3 * cellW, y, w - 3 * cellW, h, "BACK", _hasBack, _hiBack, _flashBack > 0);
}

void ButtonBar::drawCell(int x, int y, int w, int h,
                         const char* label,
                         bool enabled,
                         bool highlight,
                         bool flash) {

    const Theme& th = _themeService.current();

    _tft.fillRect(x, y, w, h, th.bg);

    uint16_t color;
    if (!enabled)          color = th.muted;
    else if (flash)        color = th.accent;        // 🔥 событие
    else if (highlight)    color = th.textSecondary; // ← спокойная подсветка
    else                   color = th.textPrimary;

    _tft.setFont(nullptr);
    _tft.setTextWrap(false);
    _tft.setTextSize(1);
    _tft.setTextColor(color, th.bg);

    int len = 0;
    for (const char* p = label; *p; ++p) len++;

    const int textW = len * 6;
    const int textH = 8;

    const int cx = x + (w - textW) / 2;
    const int cy = y + (h - textH) / 2;

    _tft.setCursor(cx, cy);
    _tft.print(label);
}