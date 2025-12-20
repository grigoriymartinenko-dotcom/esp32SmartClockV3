#include "services/LayoutService.h"

LayoutService::LayoutService(Adafruit_ST7735& tft)
: _tft(tft)
{}

void LayoutService::begin() {
    // геометрия считается динамически
}

// ===== FLAGS =====
void LayoutService::setHasBottomBar(bool v) {
    _hasBottomBar = v;
}

bool LayoutService::hasBottomBar() const {
    return _hasBottomBar;
}

// ===== HEIGHTS =====
int LayoutService::statusH() const {
    return STATUS_H;
}

int LayoutService::bottomH() const {
    return _hasBottomBar ? BOTTOM_H : 0;
}

int LayoutService::clockH() const {
    int h = _tft.height() - STATUS_H;
    if (_hasBottomBar) {
        h -= BOTTOM_H;
    }
    return h;
}

// ===== Y POSITIONS =====
int LayoutService::statusY() const {
    return 0;
}

int LayoutService::clockY() const {
    return STATUS_H;
}

int LayoutService::bottomY() const {
    return _tft.height() - BOTTOM_H;
}

// ===== SEPARATORS =====
// ЛОГИЧЕСКАЯ линия (физическую толщину рисует UiSeparator)
int LayoutService::sepStatusY() const {
    return STATUS_H;
}

int LayoutService::sepBottomY() const {
    if (!_hasBottomBar) {
        return -1;
    }
    return bottomY();
}

// ===== SAFE CLOCK RECT =====
// ClockScreen НИКОГДА не рисует вплотную к линиям
int LayoutService::clockSafeY() const {
    // 2px зарезервированы под линию Status
    return sepStatusY() + 2;
}

int LayoutService::clockSafeH() const {
    int top = clockSafeY();

    if (_hasBottomBar) {
        // 2px зарезервированы под нижнюю линию
        return sepBottomY() - 2 - top;
    }

    return _tft.height() - top;
}