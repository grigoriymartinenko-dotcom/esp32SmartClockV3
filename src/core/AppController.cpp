#include "core/AppController.h"
#include "input/Buttons.h"

/*
 * AppController.cpp
 * -----------------
 * UX-контракт навигации и роутинга кнопок.
 *
 * ВАЖНОЕ исправление для листания:
 *  - раньше SHORT LEFT на Forecast возвращал на Clock,
 *    из-за этого ForecastScreen.onShortLeft/onShortRight не вызывались.
 *  - теперь SHORT LEFT/RIGHT в Forecast идут ВНУТРЬ ForecastScreen (листание дней)
 *  - назад на Clock: LONG BACK
 */

AppController::AppController(
    ScreenManager&   sm,
    ClockScreen&     clock,
    ForecastScreen&  forecast,
    SettingsScreen&  settings
)
    : _sm(sm)
    , _clock(clock)
    , _forecast(forecast)
    , _settings(settings)
{
    _active = ActiveScreen::CLOCK;
}

void AppController::begin() {
    _active = ActiveScreen::CLOCK;
    _sm.set(_clock);
}

void AppController::goClock() {
    _active = ActiveScreen::CLOCK;
    _sm.set(_clock);
}

void AppController::goForecast() {
    _active = ActiveScreen::FORECAST;
    _sm.set(_forecast);
}

void AppController::goSettings() {
    _active = ActiveScreen::SETTINGS;
    _sm.set(_settings);
}

void AppController::handleEvent(const ButtonEvent& e) {

    // =========================================================
    // 0) ГЛОБАЛЬНОЕ: LONG OK -> Settings отовсюду (кроме Settings)
    // =========================================================
    if (e.type == ButtonEventType::LONG_PRESS &&
        e.id   == ButtonId::OK &&
        _active != ActiveScreen::SETTINGS) {

        goSettings();
        return;
    }

    // =========================================================
    // 1) SETTINGS
    // =========================================================
    if (_active == ActiveScreen::SETTINGS) {

        if (e.type == ButtonEventType::SHORT_PRESS) {
            switch (e.id) {
                case ButtonId::LEFT:  _settings.onShortLeft();  break;
                case ButtonId::RIGHT: _settings.onShortRight(); break;
                case ButtonId::OK:    _settings.onShortOk();    break;
                case ButtonId::BACK:  _settings.onShortBack();  break;
            }
        }

        if (e.type == ButtonEventType::LONG_PRESS) {
            switch (e.id) {
                case ButtonId::OK:   _settings.onLongOk();   break;
                case ButtonId::BACK: _settings.onLongBack(); break;
                default: break;
            }
        }

        if (_settings.exitRequested()) {
            _settings.clearExitRequest();
            goClock();
        }

        return;
    }

    // =========================================================
    // 2) CLOCK
    // =========================================================
    if (_active == ActiveScreen::CLOCK) {

        // SHORT RIGHT -> Forecast
        if (e.type == ButtonEventType::SHORT_PRESS &&
            e.id   == ButtonId::RIGHT) {

            goForecast();
            return;
        }

        return;
    }

    // =========================================================
    // 3) FORECAST
    // =========================================================
    if (_active == ActiveScreen::FORECAST) {

        // SHORT LEFT/RIGHT -> листание дней ВНУТРИ ForecastScreen
        if (e.type == ButtonEventType::SHORT_PRESS) {
            if (e.id == ButtonId::LEFT) {
                _forecast.onShortLeft();
                return;
            }
            if (e.id == ButtonId::RIGHT) {
                _forecast.onShortRight();
                return;
            }
        }

        // LONG BACK -> назад на Clock (универсальный "выход")
        if (e.type == ButtonEventType::LONG_PRESS &&
            e.id   == ButtonId::BACK) {

            goClock();
            return;
        }

        return;
    }
}