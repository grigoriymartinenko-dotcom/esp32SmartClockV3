#include "services/LayoutService.h"

LayoutService::LayoutService(Adafruit_ST7735& tft)
    : _tft(tft)
{
}

void LayoutService::begin() {
    // геометрия считается динамически
}

// ===== FLAGS =====

void LayoutService::setHasStatusBar(bool v) {
    _hasStatusBar = v;
}

void LayoutService::setHasButtonBar(bool v) {
    _hasButtonBar = v;
}

bool LayoutService::hasStatusBar() const {
    return _hasStatusBar;
}

bool LayoutService::hasButtonBar() const {
    return _hasButtonBar;
}

// ===== NEW HEIGHTS =====

int LayoutService::statusBarH() const {
    return _hasStatusBar ? STATUS_BAR_HEIGHT : 0;
}

int LayoutService::buttonBarH() const {
    return _hasButtonBar ? BUTTON_BAR_HEIGHT : 0;
}

int LayoutService::contentH() const {
    return _tft.height()
         - statusBarH()
         - buttonBarH();
}

// ===== NEW Y =====

int LayoutService::statusBarY() const {
    return 0;
}

int LayoutService::contentY() const {
    return statusBarH();
}

int LayoutService::buttonBarY() const {
    return _tft.height() - buttonBarH();
}