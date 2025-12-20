#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"
#include "services/LayoutService.h"
#include "services/DhtService.h"

/*
 * BottomBar
 * ---------
 * Нижняя информационная панель (температура / влажность).
 *
 * ПРАВИЛА:
 *  - НЕТ таймеров
 *  - НЕТ millis()
 *  - Рисует ТОЛЬКО через update()
 *  - Все события приходят СНАРУЖИ
 */
class BottomBar {
public:
    BottomBar(
        Adafruit_ST7735& tft,
        ThemeService& themeService,
        LayoutService& layoutService,
        DhtService& dhtService
    );

    // 🔹 реактивное обновление
    void update();

    // 🔹 события
    void markDirty();              // данные / тема изменились
    void setVisible(bool visible); // экран показал / скрыл BottomBar

private:
    void clear();
    void drawContent();

    Adafruit_ST7735& _tft;
    ThemeService&    _themeService;
    LayoutService&  _layout;
    DhtService&     _dht;

    bool _visible     = true;
    bool _wasVisible  = false;
    bool _dirty       = true;
};