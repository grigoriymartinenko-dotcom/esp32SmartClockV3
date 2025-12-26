#pragma once
#include <Adafruit_ST7735.h>
#include "services/ThemeService.h"
#include "services/LayoutService.h"

/*
 * UiSeparator
 * -----------
 * Горизонтальная разделительная линия (1px).
 *
 * Правила:
 *  - Может быть включена/выключена (visible)
 *  - Должна уметь очистить СТАРОЕ место:
 *      - при выключении
 *      - при смене y
 *      - при уходе y в -1
 *  - Не является "overlay" над контентом экрана.
 */

class UiSeparator {
public:
    UiSeparator(
        Adafruit_ST7735& tft,
        ThemeService& themeService,
        LayoutService& layoutService
    );

    void setY(int y);
    void setVisible(bool visible);
    void markDirty();
    void update();

private:
    void draw();
    void clearAt(int y);

private:
    Adafruit_ST7735& _tft;
    ThemeService&    _theme;
    LayoutService&   _layout;

    int  _y = -1;          // текущая логическая позиция
    int  _lastY = -1;      // где рисовали в прошлый раз (для корректной очистки)

    bool _visible = true;
    bool _wasVisible = false;
    bool _dirty = true;
};