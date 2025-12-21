#pragma once
#include <stdint.h>

#include "core/ScreenManager.h"
#include "input/Buttons.h"
#include "screens/ClockScreen.h"
#include "screens/ForecastScreen.h"
#include "screens/SettingsScreen.h"

/*
 * AppController
 * -------------
 * Маршрутизатор событий ввода на экраны и навигацию.
 *
 * Правила:
 *  - ScreenManager переключает экраны
 *  - SettingsScreen управляет своим меню/подменю
 *  - LONG OK  = enter submenu (или открыть Settings)
 *  - LONG BACK= выйти назад/из Settings
 */

class AppController {
public:
    AppController(
        ScreenManager& sm,
        ClockScreen& clock,
        ForecastScreen& forecast,
        SettingsScreen& settings
    );

    void begin();
    void handleEvent(const ButtonEvent& e);

private:
    enum class ActiveScreen : uint8_t {
        CLOCK = 0,
        FORECAST,
        SETTINGS
    };

    void goClock();
    void goForecast();
    void goSettings();

private:
    ScreenManager& _sm;
    ClockScreen& _clock;
    ForecastScreen& _forecast;
    SettingsScreen& _settings;

    ActiveScreen _active = ActiveScreen::CLOCK;
};