#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"
#include "layout/LayoutService.h"
#include "services/DhtService.h"

/*
 * BottomBar
 * ---------
 * 🌡 Температура / 💧 Влажность
 */
class BottomBar {
public:
    BottomBar(
        Adafruit_ST7735& tft,
        ThemeService& theme,
        LayoutService& layout,
        DhtService& dht
    );

    void draw();
    void markDirty();

private:
    Adafruit_ST7735& _tft;
    ThemeService&    _theme;
    LayoutService&  _layout;
    DhtService&     _dht;

    bool _dirty = true;
};