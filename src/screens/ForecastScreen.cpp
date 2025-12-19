#include "screens/ForecastScreen.h"
#include <Fonts/FreeSans9pt7b.h>
#include "ForecastScreen.h"

/*
 * ============================================================
 * ForecastScreen
 *
 * ВАЖНО:
 * - Экран НЕ чистит статусбар
 * - Работает ТОЛЬКО ниже STATUS_BAR_H
 * ============================================================
 */

static constexpr int STATUS_BAR_H = 24;

ForecastScreen::ForecastScreen(
    Adafruit_ST7735& tft,
    ThemeService& theme,
    ForecastService& forecast
)
: Screen(theme)
, _tft(tft)
, _forecast(forecast)
{}

void ForecastScreen::begin() {
    _hasEverBeenReady = false;
    _needsRedraw = true;
}

void ForecastScreen::update() {
    bool ready = _forecast.isReady();

    // --------------------------------------------------------
    // Пока прогноз НИ РАЗУ не был получен
    // --------------------------------------------------------
    if (!_hasEverBeenReady) {
        if (!ready) {
            if (_needsRedraw) {
                drawLoading();
                _needsRedraw = false;
            }
            return;
        }

        // первый успешный прогноз
        _hasEverBeenReady = true;
        _needsRedraw = true;
    }

    // --------------------------------------------------------
    // Прогноз уже был — рисуем его
    // --------------------------------------------------------
    if (_needsRedraw) {
        drawForecast();
        _needsRedraw = false;
    }
}

void ForecastScreen::onThemeChanged() {
    _needsRedraw = true;
}

/*
 * ------------------------------------------------------------
 * ОЧИСТКА ТОЛЬКО ОБЛАСТИ ЭКРАНА
 * НИЖЕ СТАТУСБАРА
 * ------------------------------------------------------------
 */
void ForecastScreen::clearContent() {
    _tft.fillRect(
        0,
        STATUS_BAR_H,
        _tft.width(),
        _tft.height() - STATUS_BAR_H,
        theme().bg
    );
}

void ForecastScreen::drawLoading() {
    clearContent();

    _tft.setFont(&FreeSans9pt7b);
    _tft.setTextColor(theme().primary, theme().bg);
    _tft.setCursor(20, STATUS_BAR_H + 40);
    _tft.print("Loading...");

    drawError();
}

void ForecastScreen::drawForecast() {
    clearContent();

    _tft.setFont(&FreeSans9pt7b);
    _tft.setTextColor(theme().primary, theme().bg);

    const ForecastDay* d = _forecast.today();
    if (!d) {
        drawError();
        return;
    }

    _tft.setCursor(10, STATUS_BAR_H + 30);
    _tft.printf("Day: %.1f C", d->tempDay);

    _tft.setCursor(10, STATUS_BAR_H + 50);
    _tft.printf("Night: %.1f C", d->tempNight);

    _tft.setCursor(10, STATUS_BAR_H + 70);
    _tft.printf("Humidity: %d%%", d->humidity);
}

void ForecastScreen::drawError() {
    const char* err = _forecast.lastError();
    if (!err || err[0] == '\0')
        return;

    _tft.setTextColor(theme().secondary, theme().bg);
    _tft.setCursor(10, STATUS_BAR_H + 100);
    _tft.print(err);
}