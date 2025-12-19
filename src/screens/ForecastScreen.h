#pragma once
#include "core/Screen.h"
#include "services/ForecastService.h"
#include <Adafruit_ST7735.h>

class ForecastScreen : public Screen {
public:
    ForecastScreen(
        Adafruit_ST7735& tft,
        ThemeService& theme,
        ForecastService& forecast
    );

    void begin() override;
    void update() override;
    void onThemeChanged() override;

    bool hasStatusBar() const override { return true; }

private:
    Adafruit_ST7735& _tft;
    ForecastService& _forecast;

    bool _hasEverBeenReady = false;
    bool _needsRedraw = true;

    void clearContent();

    void drawLoading();
    void drawForecast();
    void drawError();   // ⬅️ НОВОЕ
};