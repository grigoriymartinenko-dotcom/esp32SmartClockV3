#include "services/LayoutService.h"

// ============================================================================
// ctor
// ============================================================================
LayoutService::LayoutService(Adafruit_ST7735& tft)
    : _tft(tft)
{
}

// ============================================================================
// lifecycle
// ============================================================================
void LayoutService::begin() {
    // БАЗОВАЯ геометрия UI
    _hasStatusBar = true;
    _hasBottomBar = false;
}

// ============================================================================
// FLAGS
// ============================================================================
void LayoutService::setHasStatusBar(bool v) {
    _hasStatusBar = v;
}

void LayoutService::setHasBottomBar(bool v) {
    _hasBottomBar = v;
}

bool LayoutService::hasStatusBar() const {
    return _hasStatusBar;
}

bool LayoutService::hasBottomBar() const {
    return _hasBottomBar;
}

// ============================================================================
// HEIGHTS
// ============================================================================
int LayoutService::statusBarH() const {
    return _hasStatusBar ? STATUS_BAR_HEIGHT : 0;
}

int LayoutService::bottomBarH() const {
    return _hasBottomBar ? BOTTOM_BAR_HEIGHT : 0;
}

int LayoutService::contentH() const {
    return _tft.height()
         - statusBarH()
         - bottomBarH();
}

// ============================================================================
// Y POSITIONS
// ============================================================================
int LayoutService::statusBarY() const {
    return 0;
}

int LayoutService::contentY() const {
    return statusBarH();
}

int LayoutService::bottomBarY() const {
    return _tft.height() - bottomBarH();
}

// ============================================================================
// HEADER
// ============================================================================
int LayoutService::headerY() const {
    return contentY();
}

int LayoutService::headerHeight() const {
    return HEADER_HEIGHT;
}

int LayoutService::headerTextY() const {
    return headerY() + HEADER_TEXT_PAD_Y;
}

// ============================================================================
// PADDING
// ============================================================================
int LayoutService::padL() const { return PAD_L; }
int LayoutService::padR() const { return PAD_R; }
int LayoutService::padT() const { return PAD_T; }
int LayoutService::padB() const { return PAD_B; }

// ============================================================================
// CONTENT BOUNDS
// ============================================================================
int LayoutService::contentLeft() const {
    return padL();
}

int LayoutService::contentRight() const {
    return _tft.width() - padR();
}

int LayoutService::contentW() const {
    return contentRight() - contentLeft();
}

// ============================================================================
// CONTENT FLOW
// ============================================================================
int LayoutService::contentTopY() const {
    // Контент начинается ПОСЛЕ header + padding
    return headerY() + headerHeight() + padT();
}

int LayoutService::contentBottomY() const {
    return bottomBarY() - padB();
}

// ============================================================================
// TEXT METRICS
// ============================================================================
int LayoutService::lineH() const {
    return LINE_HEIGHT;
}