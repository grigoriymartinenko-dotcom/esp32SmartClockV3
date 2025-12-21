#include "core/AppController.h"

AppController::AppController(
    ScreenManager& sm,
    ClockScreen& clock,
    ForecastScreen& forecast,
    SettingsScreen& settings
)
    : _sm(sm)
    , _clock(clock)
    , _forecast(forecast)
    , _settings(settings)
{
}

void AppController::begin() {
    goClock();
}

void AppController::goClock() {
    _sm.set(_clock);
    _active = ActiveScreen::CLOCK;
}

void AppController::goForecast() {
    _sm.set(_forecast);
    _active = ActiveScreen::FORECAST;
}

void AppController::goSettings() {
    _settings.clearExitRequest();
    _sm.set(_settings);
    _active = ActiveScreen::SETTINGS;
}

void AppController::handleEvent(const ButtonEvent& e) {

    // ==================================================
    // SETTINGS screen: отдаём управление SettingsScreen
    // ==================================================
    if (_active == ActiveScreen::SETTINGS) {

        if (e.type == ButtonEventType::SHORT_PRESS) {
            if (e.id == ButtonId::LEFT)  _settings.onLeft();
            if (e.id == ButtonId::RIGHT) _settings.onRight();
            if (e.id == ButtonId::OK)    _settings.onOk();
            if (e.id == ButtonId::BACK)  _settings.onBack();
        }

        // LONG BACK = выход назад/из Settings (классический UX)
        if (e.type == ButtonEventType::LONG_PRESS && e.id == ButtonId::BACK) {
            _settings.onBackLong();  // ✅ добавим метод в SettingsScreen
        }

        // LONG OK = вход в подменю (если применимо)
        if (e.type == ButtonEventType::LONG_PRESS && e.id == ButtonId::OK) {
            _settings.onOkLong();    // ✅ добавим метод в SettingsScreen
        }

        // универсальный выход по флагу
        if (_settings.exitRequested()) {
            _settings.clearExitRequest();
            goClock();
        }

        return;
    }

    // ==================================================
    // НЕ settings: быстрые действия
    // ==================================================

    // LONG OK = вход в Settings (удобно из любого экрана)
    if (e.type == ButtonEventType::LONG_PRESS && e.id == ButtonId::OK) {
        goSettings();
        return;
    }

    // SHORT PRESS логика как была
    if (e.type == ButtonEventType::SHORT_PRESS) {
        if (e.id == ButtonId::LEFT)  goForecast();
      //  if (e.id == ButtonId::RIGHT) goClock();
        //if (e.id == ButtonId::OK)    goSettings(); // короткий OK тоже открывает (если хочешь убрать — скажешь)
        if (e.id == ButtonId::BACK)  goClock();
    }
}