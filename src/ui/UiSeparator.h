#pragma once
#include <Adafruit_ST7735.h>
#include "services/ThemeService.h"

/*
 * UiSeparator
 * -----------
 * Горизонтальный разделитель зон интерфейса
 */
class UiSeparator {
public:
    UiSeparator(
        Adafruit_ST7735& tft,
        ThemeService& theme,
        int y
    );

    void draw();
    void markDirty();

    void setY(int y);

private:
    Adafruit_ST7735& _tft;
    ThemeService&   _theme;

    int  _y;
    bool _dirty = true;
};