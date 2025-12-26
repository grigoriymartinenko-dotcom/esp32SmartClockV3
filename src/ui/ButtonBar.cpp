#include "ui/ButtonBar.h"
#include <Adafruit_GFX.h>

static constexpr int BASELINE_SHIFT = -2;

ButtonBar::ButtonBar(
    Adafruit_ST7735& tft,
    ThemeService& themeService,
    LayoutService& layoutService
)
    : _tft(tft)
    , _themeService(themeService)
    , _layout(layoutService)
{
    // SAFE DEFAULT — чтобы ButtonBar не была пустой
    _labelLeft  = "<";
    _labelOk    = "OK";
    _labelRight = ">";
    _labelBack  = "BACK";
}

// ============================================================================
// Public API
// ============================================================================
void ButtonBar::markDirty() {
    _dirty = true;
}

void ButtonBar::setVisible(bool visible) {
    if (_visible == visible) return;
    _visible = visible;
    _dirty = true;
}

void ButtonBar::setActions(bool left, bool ok, bool right, bool back) {
    _hasLeft  = left;
    _hasOk    = ok;
    _hasRight = right;
    _hasBack  = back;
    _dirty = true;
}

void ButtonBar::setHighlight(bool left, bool ok, bool right, bool back) {
    _hiLeft  = left;
    _hiOk    = ok;
    _hiRight = right;
    _hiBack  = back;
    _dirty = true;
}

void ButtonBar::setLabels(
    const char* left,
    const char* ok,
    const char* right,
    const char* back
) {
    _labelLeft  = left;
    _labelOk    = ok;
    _labelRight = right;
    _labelBack  = back;
    _dirty = true;
}

void ButtonBar::flash(ButtonId id) {
    switch (id) {
        case ButtonId::LEFT:  _flashLeft  = FLASH_FRAMES; break;
        case ButtonId::OK:    _flashOk    = FLASH_FRAMES; break;
        case ButtonId::RIGHT: _flashRight = FLASH_FRAMES; break;
        case ButtonId::BACK:  _flashBack  = FLASH_FRAMES; break;
    }
    _dirty = true;
}

bool ButtonBar::anyFlashActive() const {
    return _flashLeft || _flashOk || _flashRight || _flashBack;
}

// ============================================================================
// Update
// ============================================================================
void ButtonBar::update() {

    const int h = _layout.buttonBarH();

    if (h != _lastBarH) {
        clear();
        _dirty = true;
        _wasVisible = false;
        _lastBarH = h;
    }

    if (h <= 0) {
        _wasVisible = false;
        return;
    }

    if (!_visible && _wasVisible) {
        clear();
        _wasVisible = false;
        return;
    }

    if (!_visible)
        return;

    if (_dirty || anyFlashActive() || !_wasVisible) {
        clear();
        draw();
        _dirty = false;
        _wasVisible = true;
    }

    if (_flashLeft)  --_flashLeft;
    if (_flashOk)    --_flashOk;
    if (_flashRight) --_flashRight;
    if (_flashBack)  --_flashBack;
}

// ============================================================================
// Drawing
// ============================================================================
void ButtonBar::clear() {
    const Theme& th = _themeService.current();

    const int h = _layout.buttonBarH();
    if (h <= 0) return;

    _tft.fillRect(
        0,
        _layout.buttonBarY(),
        _tft.width(),
        h,
        th.bg
    );
}

void ButtonBar::draw() {

    const int y = _layout.buttonBarY();
    const int h = _layout.buttonBarH();
    const int w = _tft.width();
    const int cellW = w / 4;

    drawCell(0 * cellW, y, cellW, h, _labelLeft,
             _hasLeft, _hiLeft, _flashLeft);

    drawCell(1 * cellW, y, cellW, h, _labelOk,
             _hasOk, _hiOk, _flashOk);

    drawCell(2 * cellW, y, cellW, h, _labelRight,
             _hasRight, _hiRight, _flashRight);

    drawCell(3 * cellW, y, cellW, h, _labelBack,
             _hasBack, _hiBack, _flashBack);
}

void ButtonBar::drawCell(
    int x, int y, int w, int h,
    const char* label,
    bool enabled,
    bool highlight,
    bool flash
) {
    const Theme& th = _themeService.current();

    uint16_t bg = th.bg;
    uint16_t fg = th.textSecondary;

    if (!enabled) {
        fg = th.textSecondary;
    } else if (flash) {
        bg = th.accent;
        fg = th.bg;
    } else if (highlight) {
        fg = th.textPrimary;
    }

    _tft.fillRect(x, y, w, h, bg);

    if (!label || !*label) return;

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextWrap(false);
    _tft.setTextColor(fg, bg);

    int16_t x1, y1;
    uint16_t tw, thh;
    _tft.getTextBounds(label, 0, 0, &x1, &y1, &tw, &thh);

    const int textX = x + (w - (int)tw) / 2;
    const int baselineY = y + (h / 2) + (thh / 2) - y1 + BASELINE_SHIFT;

    _tft.setCursor(textX, baselineY);
    _tft.print(label);
}