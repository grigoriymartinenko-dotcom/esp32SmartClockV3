#include "core/AppController.h"
#include "input/Buttons.h"

/*
 * AppController.cpp
 * -----------------
 * ЕДИНЫЙ роутер кнопок.
 *
 * ПРАВИЛО:
 *  - AppController решает ТОЛЬКО куда идёт событие
 *  - Экран решает, что оно значит
 *
 * ВАЖНО (фикс Wi-Fi Scan):
 *  - В SettingsScreen есть Wi-Fi-специфичный обработчик OK: handleWifiShortOk()
 *  - Он должен иметь приоритет над обычным onShortOk(),
 *    иначе OK в Wi-Fi списке/скане будет трактоваться как enterEdit().
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
    // GLOBAL: LONG OK -> Settings (из любого экрана)
    // =========================================================
    if (e.type == ButtonEventType::LONG_PRESS &&
        e.id   == ButtonId::OK &&
        _active != ActiveScreen::SETTINGS) {

        goSettings();
        return;
    }
    // =========================================================
    // SETTINGS
    // =========================================================
    if (_active == ActiveScreen::SETTINGS) {

        // -----------------------------------------------------
        // LONG PRESS — всегда разрешены
        // -----------------------------------------------------
        if (e.type == ButtonEventType::LONG_PRESS) {
            switch (e.id) {
                case ButtonId::OK:   _settings.onLongOk();   break;
                case ButtonId::BACK: _settings.onLongBack(); break;
                default: break;
            }
        }

        // -----------------------------------------------------
        // SHORT PRESS
        // -----------------------------------------------------
        if (e.type == ButtonEventType::SHORT_PRESS) {
            switch (e.id) {
                case ButtonId::LEFT:
                    _settings.onShortLeft();
                    break;

                case ButtonId::RIGHT:
                    _settings.onShortRight();
                    break;

                case ButtonId::OK:
                    // 🔑 FIX: Wi-Fi OK имеет приоритет.
                    // Если SettingsScreen сейчас в Wi-Fi контексте (scan/list/password),
                    // он обработает OK сам и вернёт true.
                    // Если нет — вернёт false, и мы идём в обычный onShortOk().
                    if (_settings.handleWifiShortOk()) {
                        // обработано Wi-Fi логикой
                        break;
                    }
                    _settings.onShortOk();
                    break;

                case ButtonId::BACK:
                    _settings.onShortBack();
                    break;
            }
        }

        // Выход из Settings обратно на Clock
        if (_settings.exitRequested()) {
            _settings.clearExitRequest();
            goClock();
        }

        return;
    }

    // =========================================================
    // CLOCK
    // =========================================================
    if (_active == ActiveScreen::CLOCK) {

        if (e.type == ButtonEventType::LONG_PRESS &&
            e.id   == ButtonId::LEFT) {

            goForecast();
        }

        return;
    }

    // =========================================================
    // FORECAST
    // =========================================================
    if (_active == ActiveScreen::FORECAST) {

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

        if (e.type == ButtonEventType::LONG_PRESS &&
            e.id   == ButtonId::BACK) {

            goClock();
            return;
        }

        return;
    }
}