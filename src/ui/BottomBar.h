#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"
#include "services/LayoutService.h"
#include "services/DhtService.h"

class BottomBar {
public:
    BottomBar(
        Adafruit_ST7735& tft,
        ThemeService& themeService,
        LayoutService& layoutService,
        DhtService& dhtService
    );

    void draw();
    void markDirty();
    void setVisible(bool visible);

private:
    void clearInternal();
    void drawContent();

private:
    Adafruit_ST7735& _tft;
    ThemeService&    _themeService;
    LayoutService&  _layout;
    DhtService&     _dht;

    bool _visible    = true;
    bool _wasVisible = true;
    bool _dirty      = true;
};