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

    if (_dirty || !_wasVisible) {
        draw();
        _dirty = false;
        _wasVisible = true;
    }
}

void ButtonBar::clear() {
    const int y = _layout.bottomY();
    const int h = _layout.bottomH();
    _tft.fillRect(0, y, _tft.width(), h, _themeService.current().bg);
}

void ButtonBar::draw() {
    const Theme& th = _themeService.current();

    const int y = _layout.bottomY();
    const int h = _layout.bottomH();

    // фон панели
    _tft.fillRect(0, y, _tft.width(), h, th.bg);

    // 4 ячейки
    const int w = _tft.width();
    const int cellW = w / 4;

    // маленькая "безопасная" внутренняя рамка через отступ текста, без магии пикселей
    drawCell(0 * cellW, y, cellW, h, "<-",   _hasLeft,  _hiLeft);
    drawCell(1 * cellW, y, cellW, h, "OK",   _hasOk,    _hiOk);
    drawCell(2 * cellW, y, cellW, h, "->",   _hasRight, _hiRight);
    drawCell(3 * cellW, y, w - 3 * cellW, h, "BACK", _hasBack, _hiBack);
}

void ButtonBar::drawCell(int x, int y, int w, int h, const char* label, bool enabled, bool highlight) {
    const Theme& th = _themeService.current();

    // подсветка = accent текст, иначе muted/primary
    uint16_t color;
    if (!enabled) color = th.muted;
    else if (highlight) color = th.accent;
    else color = th.textPrimary;

    _tft.setFont(nullptr);
    _tft.setTextWrap(false);
    _tft.setTextSize(1);
    _tft.setTextColor(color, th.bg);

    // центрирование текста (приближённо, без getTextBounds → проще и быстрее)
    // символы у default font ~6px ширина. Это достаточно стабильно для наших коротких лейблов.
    int len = 0;
    for (const char* p = label; *p; ++p) len++;

    const int textW = len * 6;
    const int textH = 8;

    const int cx = x + (w - textW) / 2;
    const int cy = y + (h - textH) / 2;

    // очистить зону текста (на случай смены label)
    _tft.fillRect(x, y, w, h, th.bg);

    _tft.setCursor(cx, cy);
    _tft.print(label);
}