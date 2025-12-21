#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"
#include "services/LayoutService.h"

/*
 * ButtonBar
 * ---------
 * Нижняя навигационная панель для действий: <- OK -> BACK
 *
 * Правила:
 *  - НЕТ millis()
 *  - НЕТ таймеров
 *  - Только реактивная перерисовка через update()
 *  - Никакой логики кнопок GPIO — только визуализация
 */

class ButtonBar {
public:
    enum class ButtonId : uint8_t {
        LEFT = 0,
        OK,
        RIGHT,
        BACK
    };

    ButtonBar(
        Adafruit_ST7735& tft,
        ThemeService& themeService,
        LayoutService& layoutService
    );

    void update();

    // видимость (например, экран включил/выключил)
    void setVisible(bool visible);

    // конфигурация доступных действий
    void setActions(bool left, bool ok, bool right, bool back);

    // подсветка (например, какой action сейчас "выбран")
    void setHighlight(bool left, bool ok, bool right, bool back);

    // мигнуть/подсветить кнопку как "нажато"
    void flash(ButtonId id);

    // принудительная перерисовка (смена темы/экрана)
    void markDirty();

private:
    void clear();
    void draw();

    void drawCell(int x, int y, int w, int h, const char* label, bool enabled, bool highlight, bool flash);

    bool anyFlashActive() const;

private:
    Adafruit_ST7735& _tft;
    ThemeService&    _themeService;
    LayoutService&   _layout;

    bool _visible     = true;
    bool _wasVisible  = false;
    bool _dirty       = true;

    bool _hasLeft  = true;
    bool _hasOk    = true;
    bool _hasRight = true;
    bool _hasBack  = true;

    bool _hiLeft  = false;
    bool _hiOk    = false;
    bool _hiRight = false;
    bool _hiBack  = false;

    // flash "нажатия" (реактивно, без millis)
    uint8_t _flashLeft  = 0;
    uint8_t _flashOk    = 0;
    uint8_t _flashRight = 0;
    uint8_t _flashBack  = 0;

    static constexpr uint8_t FLASH_FRAMES = 6; // было 2
};