#include "services/LayoutService.h"

LayoutService::LayoutService(Adafruit_ST7735& tft)
: _tft(tft)
{}

void LayoutService::begin() {
    _clockH = _tft.height() - STATUS_H - BOTTOM_H;
}

// ===== HEIGHTS =====
int LayoutService::statusH() const { return STATUS_H; }
int LayoutService::clockH()  const { return _clockH; }
int LayoutService::bottomH() const { return BOTTOM_H; }

// ===== Y POSITIONS =====
int LayoutService::statusY() const { return 0; }
int LayoutService::clockY()  const { return STATUS_H; }
int LayoutService::bottomY() const { return _tft.height() - BOTTOM_H; }

// ===== SEPARATORS (2px OFFSET) =====
int LayoutService::sepStatusY() const {
    // линия чуть ниже нижней границы статусбара
    return STATUS_H + SEP_OFFSET;
}

int LayoutService::sepBottomY() const {
    // линия чуть выше верхней границы нижнего бара
    return bottomY() - SEP_OFFSET;
}

// ===== SAFE CLOCK RECT =====
int LayoutService::clockSafeY() const {
    // ниже верхней линии на 1px (чтобы не затирать её)
    return sepStatusY() + 1;
}

int LayoutService::clockSafeH() const {
    // до нижней линии -1px
    return (sepBottomY() - 1) - clockSafeY();
}