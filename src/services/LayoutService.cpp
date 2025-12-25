#include "services/LayoutService.h"

LayoutService::LayoutService(Adafruit_ST7735& tft)
    : _tft(tft)
{
}

void LayoutService::begin() {
}

// ===== FLAGS =====

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

// ===== HEIGHTS =====

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

// ===== Y POSITIONS =====

int LayoutService::statusBarY() const {
    return 0;
}

int LayoutService::contentY() const {
    return statusBarH();
}

int LayoutService::bottomBarY() const {
    return _tft.height() - bottomBarH();
}