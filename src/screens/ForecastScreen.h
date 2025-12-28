#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/ThemeService.h"
#include "services/ForecastService.h"
#include "services/LayoutService.h"
#include "services/UiVersionService.h"

#include "ui/weather/WeatherIcons.h"

/*
 * ForecastScreen
 * --------------
 * Экран прогноза погоды.
 *
 * ПРИНЦИП:
 *  - Экран НИЧЕГО не знает про Day/Night
 *  - Экран использует ТОЛЬКО ThemeBlend
 *  - Экран реагирует ТОЛЬКО на UiVersionService
 *
 * Все решения о цветах и плавности переходов
 * принимаются ВНЕ экрана (ThemeService + NightTransitionService + ColorTemp).
 *
 * UX:
 *  - перелистывание дней с анимацией (slide + лёгкий fade)
 */
class ForecastScreen : public Screen {
public:
    ForecastScreen(
        Adafruit_ST7735&  tft,
        ThemeService&     theme,
        ForecastService&  forecast,
        LayoutService&    layout,
        UiVersionService& ui
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
    // ---- draw helpers ----
    void redrawAll();

    // Рисование готового состояния (одного дня) с X-смещением.
    // xOff может быть отрицательным/положительным для slide-анимации.
    void drawReadyAtX(const ThemeBlend& b, const ForecastDay* d,
                      uint8_t idx, uint8_t total, int xOff);

    void drawHeaderAtX(const ThemeBlend& b,
                       const ForecastDay* d,
                       uint8_t idx,
                       uint8_t total,
                       int xOff);

    void drawLoading(const ThemeBlend& b);
    void drawError(const ThemeBlend& b);

    void drawRowDayAtX(const ThemeBlend& b, const ForecastDay* d, int xOff);
    void drawRowNightAtX(const ThemeBlend& b, const ForecastDay* d, int xOff);
    void drawRowHumAtX(const ThemeBlend& b, int hum, int xOff);

    // ---- animation ----
    void startDayTransition(int dir);        // dir: -1 (left), +1 (right)
    void drawTransitionFrame(const ThemeBlend& b);

private:
    Adafruit_ST7735&  _tft;
    ForecastService&  _forecast;
    LayoutService&    _layout;
    UiVersionService& _ui;

    UiState _state     = UiState::LOADING;
    UiState _lastState = UiState::ERROR;

    uint8_t _dayIndex     = 0;
    uint8_t _lastDayIndex = 255;

    bool _dirty = true;

    // ---- animation state ----
    bool     _animActive   = false;
    uint32_t _animStartMs  = 0;
    uint8_t  _animFrom     = 0;
    uint8_t  _animTo       = 0;
    int      _animDir      = 0; // -1 or +1

    // Длительность анимации (мс). 180..240 обычно выглядит отлично.
    static constexpr uint16_t ANIM_MS = 200;
};