#pragma once
#include <stdint.h>

#include "input/Buttons.h"
#include "core/ScreenManager.h"
#include "screens/ClockScreen.h"
#include "screens/ForecastScreen.h"
#include "screens/SettingsScreen.h"

/*
 * AppController
 * -------------
 * Контроллер приложения (НЕ UI).
 *
 * Ответственность:
 *  - знает, какой экран активен
 *  - маршрутизирует кнопки
 *  - выполняет переходы между экранами
 *
 * НЕ делает:
 *  - не рисует
 *  - не знает про темы
 *  - не знает про Wi-Fi / NTP
 */

class AppController {
public:
    AppController(
        ScreenManager& screenManager,
        ClockScreen& clock,
        ForecastScreen& forecast,
        SettingsScreen& settings
    );

    void begin();
    void handleButtons(const ButtonsState& btn);

private:
    enum class ActiveScreen : uint8_t {
        CLOCK = 0,
        FORECAST,
        SETTINGS
    };

private:
    void goClock();
    void goForecast();
    void goSettings();

private:
    ScreenManager&  _screenManager;
    ClockScreen&    _clock;
    ForecastScreen& _forecast;
    SettingsScreen& _settings;

    ActiveScreen _active = ActiveScreen::CLOCK;
};