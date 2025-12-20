#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"

/*
 * UiSeparator
 * -----------
 * Горизонтальная линия-разделитель UI.
 *
 * ПРАВИЛА:
 *  - НЕТ таймеров
 *  - НЕТ millis()
 *  - Рисует ТОЛЬКО через update()
 *  - y < 0  => линия отключена
 */
class UiSeparator {
public:
    UiSeparator(
        Adafruit_ST7735& tft,
        ThemeService& theme,
        int y
    );

    // 🔹 изменить позицию линии
    void setY(int y);

    // 🔹 реактивное обновление
    void update();

    // 🔹 принудительно пометить на перерисовку
    void markDirty();

private:
    void draw();

    Adafruit_ST7735& _tft;
    ThemeService&    _theme;

    int  _y = -1;
    bool _dirty = true;
};