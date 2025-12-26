#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"
#include "services/LayoutService.h"

/*
 * ButtonBar
 * ---------
 * Визуальная панель кнопок.
 *
 * Принципы:
 *  - ButtonBar НЕ знает смысла кнопок
 *  - Экран задаёт подписи и состояния
 *  - ButtonBar только рисует
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

    // visibility / state
    void setVisible(bool visible);
    void setActions(bool left, bool ok, bool right, bool back);
    void setHighlight(bool left, bool ok, bool right, bool back);

    // 🔑 ШАГ 2.2 — контекстные подписи
    void setLabels(
        const char* left,
        const char* ok,
        const char* right,
        const char* back
    );

    void flash(ButtonId id);
    void markDirty();

private:
    void clear();
    void draw();
    void drawCell(
        int x, int y, int w, int h,
        const char* label,
        bool enabled,
        bool highlight,
        bool flash
    );

    bool anyFlashActive() const;

private:
    Adafruit_ST7735& _tft;
    ThemeService&    _themeService;
    LayoutService&   _layout;

    bool _visible    = true;
    bool _wasVisible = false;
    bool _dirty      = true;
    int  _lastBarH   = -1;

    bool _hasLeft  = true;
    bool _hasOk    = true;
    bool _hasRight = true;
    bool _hasBack  = true;

    bool _hiLeft  = false;
    bool _hiOk    = false;
    bool _hiRight = false;
    bool _hiBack  = false;

    const char* _labelLeft  = nullptr;
    const char* _labelOk    = nullptr;
    const char* _labelRight = nullptr;
    const char* _labelBack  = nullptr;

    uint8_t _flashLeft  = 0;
    uint8_t _flashOk    = 0;
    uint8_t _flashRight = 0;
    uint8_t _flashBack  = 0;

    static constexpr uint8_t FLASH_FRAMES = 6;
};