#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/ForecastService.h"
#include "services/LayoutService.h"

/*
 * ForecastScreen
 * --------------
 * Экран прогноза погоды (FREE API).
 * Показывает ТОЛЬКО сегодняшний день.
 */
class ForecastScreen : public Screen {
public:
    ForecastScreen(
        Adafruit_ST7735& tft,
        ThemeService& theme,
        ForecastService& forecast,
        LayoutService& layout
    );

    void begin() override;
    void update() override;
    
    bool hasBottomBar() const override { return false; }
    bool hasStatusBar() const override;

private:
    void drawForecast();

    Adafruit_ST7735& _tft;
    ForecastService& _forecast;
    LayoutService&   _layout;

    bool _dirty = true;
};