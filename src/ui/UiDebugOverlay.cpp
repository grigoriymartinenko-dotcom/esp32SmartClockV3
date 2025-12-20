#include "ui/UiDebugOverlay.h"
#include <Arduino.h>

bool UiDebugOverlay::_enabled = false;

void UiDebugOverlay::setEnabled(bool e) {
    _enabled = e;
}

bool UiDebugOverlay::isEnabled() {
    return _enabled;
}

void UiDebugOverlay::draw(Adafruit_ST7735& tft) {
    if (!_enabled)
        return;

    // небольшая подложка в левом верхнем углу
    constexpr int W = 128;
    constexpr int H = 48;

    tft.fillRect(0, 0, W, H, ST7735_BLACK);

    tft.setFont(nullptr);
    tft.setTextSize(1);
    tft.setTextWrap(false);
    tft.setTextColor(ST7735_GREEN, ST7735_BLACK);

    int y = 4;

    tft.setCursor(4, y);
    tft.print("DEBUG");
    y += 10;

    tft.setCursor(4, y);
    tft.printf("Heap: %u", ESP.getFreeHeap());
    y += 10;

    tft.setCursor(4, y);
    tft.printf("Uptime: %lus", millis() / 1000);
}