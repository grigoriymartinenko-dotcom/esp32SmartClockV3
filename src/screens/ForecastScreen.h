#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/ThemeService.h"
#include "services/ForecastService.h"
#include "services/LayoutService.h"

/*
 * ForecastScreen
 * --------------
 * Реактивный экран прогноза (до 5 дней).
 *
 * UX:
 *  - SHORT LEFT/RIGHT -> листание дней
 *  - LONG BACK        -> Clock (AppController)
 *  - LONG OK          -> Settings (глобально)
 *
 * Состояния:
 *  - LOADING  -> прогноз ещё загружается
 *  - READY    -> данные есть
 *  - ERROR    -> ошибка (WiFi / HTTP / JSON)
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
    bool hasBottomBar() const override { return false; }

    // вызывается AppController
    void onShortLeft();
    void onShortRight();

private:
    enum class UiState : uint8_t {
        LOADING,
        READY,
        ERROR
    };

private:
    // draw helpers
    void drawHeader(bool force, const ForecastDay* d, uint8_t idx, uint8_t total);
    void drawLoading(bool force);
    void drawError(bool force);
    void drawRowDay(bool force, int dayTemp);
    void drawRowNight(bool force, int nightTemp);
    void drawRowHum(bool force, int hum);

    // utils
    void clearWorkArea();
    //void hardClearBottom2px();
    void resetCache();
    bool themeChanged() const;

private:
    Adafruit_ST7735& _tft;
    ForecastService& _forecast;
    LayoutService&   _layout;

    bool _dirty = true;

    UiState _state = UiState::LOADING;
    UiState _lastState = UiState::ERROR;

    uint8_t _dayIndex = 0;
    uint8_t _lastDayIndex = 255;

    int  _lastDay   = -10000;
    int  _lastNight = -10000;
    int  _lastHum   = -1;

    uint16_t _lastBg = 0;
};