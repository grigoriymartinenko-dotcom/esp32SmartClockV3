#include "core/AppController.h"

AppController::AppController(
    ScreenManager& screenManager,
    ClockScreen& clock,
    ForecastScreen& forecast,
    SettingsScreen& settings
)
    : _screenManager(screenManager)
    , _clock(clock)
    , _forecast(forecast)
    , _settings(settings)
{
}

void AppController::begin() {
    _screenManager.set(_clock);
    _active = ActiveScreen::CLOCK;
}

// =====================================================
// Navigation helpers
// =====================================================
void AppController::goClock() {
    _screenManager.set(_clock);
    _active = ActiveScreen::CLOCK;
}

void AppController::goForecast() {
    _screenManager.set(_forecast);
    _active = ActiveScreen::FORECAST;
}

void AppController::goSettings() {
    _settings.clearExitRequest();
    _screenManager.set(_settings);
    _active = ActiveScreen::SETTINGS;
}

// =====================================================
// Buttons routing
// =====================================================
void AppController::handleButtons(const ButtonsState& btn) {

    if (_active == ActiveScreen::SETTINGS) {

        // ===== SETTINGS =====
        if (btn.left)  _settings.onLeft();
        if (btn.right) _settings.onRight();
        if (btn.ok)    _settings.onOk();
        if (btn.back)  _settings.onBack();

        // выход из Settings по флагу
        if (_settings.exitRequested()) {
            _settings.clearExitRequest();
            goClock();
        }

        return;
    }

    // ===== НЕ SETTINGS =====
    // LEFT  -> Forecast
    // RIGHT -> Clock
    // OK    -> Settings
    // BACK  -> Clock

    if (btn.left)  goForecast();
    if (btn.right) goClock();
    if (btn.ok)    goSettings();
    if (btn.back)  goClock();
}