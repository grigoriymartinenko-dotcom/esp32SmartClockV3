#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/ThemeService.h"
#include "services/ForecastService.h"
#include "services/LayoutService.h"

#include "ui/weather/WeatherIcons.h"

/*
 * ForecastScreen (SAFE MODE)
 * -------------------------
 * Стабильная версия без NightTransition / ThemeBlend.
 *
 * Цель:
 *  - убрать мусор "4545646545"
 *  - убрать мигание
 *  - гарантировать целостный рендер
 *
 * Красоту вернём ПОСЛЕ стабилизации сервиса.
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

    bool hasStatusBar() const override { return true; }

    void onShortLeft();
    void onShortRight();

private:
    enum class UiState : uint8_t {
        LOADING,
        READY,
        ERROR
    };

private:
    void drawHeader(bool force, const ForecastDay* d, uint8_t idx, uint8_t total);
    void drawLoading(bool force);
    void drawError(bool force);
    void drawRowDay(bool force, const ForecastDay* d);
    void drawRowNight(bool force, const ForecastDay* d);
    void drawRowHum(bool force, int hum);

    void clearWorkArea();

private:
    Adafruit_ST7735& _tft;
    ForecastService& _forecast;
    LayoutService&   _layout;

    UiState _state = UiState::LOADING;
    UiState _lastState = UiState::ERROR;

    uint8_t _dayIndex = 0;
    uint8_t _lastDayIndex = 255;

    bool _dirty = true;
};