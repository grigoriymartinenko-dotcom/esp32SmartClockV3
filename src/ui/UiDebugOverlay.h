#pragma once
#include <Adafruit_ST7735.h>

/*
 * UiDebugOverlay
 * --------------
 * Полупрозрачный debug-overlay поверх UI.
 * Рисуется ПОСЛЕДНИМ.
 */
class UiDebugOverlay {
public:
    static void setEnabled(bool e);
    static bool isEnabled();

    static void draw(Adafruit_ST7735& tft);

private:
    static bool _enabled;
};