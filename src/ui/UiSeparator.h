#pragma once
#include <Adafruit_ST7735.h>
#include "services/ThemeService.h"
#include "services/LayoutService.h"

/*
 * UiSeparator
 * -----------
 * Горизонтальная разделительная линия.
 *
 * ПРАВИЛА:
 *  - Может быть ВКЛЮЧЕНА или ВЫКЛЮЧЕНА
 *  - При выключении ОБЯЗАТЕЛЬНО очищает свою зону
 *  - LayoutService задаёт ТОЛЬКО логическую Y-позицию
 */

class UiSeparator {
public:
    UiSeparator(
        Adafruit_ST7735& tft,
        ThemeService& themeService,
        LayoutService& layoutService
    );

    // установить логическую позицию линии
    void setY(int y);

    // 🔴 НОВОЕ: управление видимостью
    void setVisible(bool visible);

    // пометить на перерисовку
    void markDirty();

    // основной апдейт
    void update();

private:
    void draw();
    void clear();

private:
    Adafruit_ST7735& _tft;
    ThemeService&    _theme;
    LayoutService&   _layout;

    int  _y = -1;
    bool _visible = true;
    bool _wasVisible = false;
    bool _dirty = true;
};