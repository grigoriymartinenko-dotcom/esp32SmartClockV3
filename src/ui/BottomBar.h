#pragma once
#include <Adafruit_ST7735.h>

#include "services/LayoutService.h"
#include "services/ThemeService.h"

/*
 * BottomBar
 * ---------
 * Универсальная нижняя панель кнопок.
 *
 * ПРИНЦИПЫ:
 *  - чистый UI (без логики кнопок)
 *  - экран ОПИСЫВАЕТ кнопки
 *  - BottomBar только РИСУЕТ
 *  - фиксированная геометрия и стиль
 */

class BottomBar {
public:
    enum class Button : uint8_t {
        LEFT = 0,
        OK,
        RIGHT,
        BACK,
        COUNT
    };

    struct ButtonState {
        const char* label = nullptr;
        bool enabled = false;
        bool highlight = false;
        uint8_t flash = 0;
    };

public:
    BottomBar(
        Adafruit_ST7735& tft,
        LayoutService& layout,
        ThemeService& theme
    );

    // visibility
    void setVisible(bool v);
    bool isVisible() const;

    // buttons API (вызывается экраном)
    void clearButtons();

    void setButton(Button id, const char* label, bool enabled = true);
    void setEnabled(Button id, bool enabled);
    void setHighlight(Button id, bool highlight);
    void flash(Button id);

    void markDirty();

    // lifecycle
    void update();

private:
    void clear();
    void draw();
    void drawButton(
        int x, int y, int w, int h,
        const ButtonState& st
    );

private:
    Adafruit_ST7735& _tft;
    LayoutService&   _layout;
    ThemeService&    _theme;

    bool _visible = false;
    bool _dirty   = true;

    ButtonState _buttons[(int)Button::COUNT];

    static constexpr uint8_t FLASH_FRAMES = 6;
};