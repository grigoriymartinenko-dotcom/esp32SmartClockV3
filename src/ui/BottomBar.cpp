#include "ui/BottomBar.h"

// ============================================================================
// ctor
// ============================================================================
BottomBar::BottomBar(
    Adafruit_ST7735& tft,
    LayoutService& layout,
    ThemeService& theme
)
: _tft(tft)
, _layout(layout)
, _theme(theme)
{
    clearButtons();
}

// ============================================================================
// visibility
// ============================================================================
void BottomBar::setVisible(bool v) {
    if (_visible == v) return;
    _visible = v;
    _dirty = true;
}

bool BottomBar::isVisible() const {
    return _visible;
}

// ============================================================================
// buttons API
// ============================================================================
void BottomBar::clearButtons() {
    for (int i = 0; i < (int)Button::COUNT; ++i) {
        _buttons[i] = ButtonState{};
    }
    _dirty = true;
}

void BottomBar::setButton(Button id, const char* label, bool enabled) {
    auto& b = _buttons[(int)id];
    b.label = label;
    b.enabled = enabled;
    _dirty = true;
}

void BottomBar::setEnabled(Button id, bool enabled) {
    auto& b = _buttons[(int)id];
    if (b.enabled == enabled) return;
    b.enabled = enabled;
    _dirty = true;
}

void BottomBar::setHighlight(Button id, bool highlight) {
    auto& b = _buttons[(int)id];
    if (b.highlight == highlight) return;
    b.highlight = highlight;
    _dirty = true;
}

void BottomBar::flash(Button id) {
    auto& b = _buttons[(int)id];
    b.flash = FLASH_FRAMES;
    _dirty = true;
}

void BottomBar::markDirty() {
    _dirty = true;
}

// ============================================================================
// update
// ============================================================================
void BottomBar::update() {

    bool anyFlash = false;
    for (auto& b : _buttons) {
        if (b.flash > 0) {
            b.flash--;
            anyFlash = true;
        }
    }

    if (!_visible) {
        if (_dirty) clear();
        _dirty = false;
        return;
    }

    if (_dirty || anyFlash) {
        draw();
        _dirty = false;
    }
}

// ============================================================================
// draw helpers
// ============================================================================
void BottomBar::clear() {
    const Theme& th = _theme.current();
    _tft.fillRect(
        0,
        _layout.bottomBarY(),
        _tft.width(),
        _layout.bottomBarH(),
        th.bg
    );
}

void BottomBar::draw() {
    const Theme& th = _theme.current();

    const int y = _layout.bottomBarY();
    const int h = _layout.bottomBarH();
    const int w = _tft.width() / (int)Button::COUNT;

    _tft.fillRect(0, y, _tft.width(), h, th.bg);

    for (int i = 0; i < (int)Button::COUNT; ++i) {
        drawButton(
            i * w,
            y,
            w,
            h,
            _buttons[i]
        );
    }
}

void BottomBar::drawButton(
    int x, int y, int w, int h,
    const ButtonState& st
) {
    if (!st.label) return;

    const Theme& th = _theme.current();

    uint16_t fg = th.textPrimary;
    uint16_t bg = th.bg;

    if (!st.enabled) {
        fg = th.muted;
    }
    if (st.highlight) {
        fg = th.accent;
    }
    if (st.flash > 0) {
        fg = th.bg;
        bg = th.accent;
    }

    _tft.fillRect(x, y, w, h, bg);

    _tft.setTextSize(1);
    _tft.setTextColor(fg, bg);
    _tft.setTextWrap(false);

    const int tx = x + (w - strlen(st.label) * 6) / 2;
    const int ty = y + (h - 8) / 2;

    _tft.setCursor(tx, ty);
    _tft.print(st.label);
}