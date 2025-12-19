#include "screens/ForecastScreen.h"
#include <math.h>

/*
 * ForecastScreen
 * --------------
 * Использует данные ForecastService (today only).
 * Рисует ТОЛЬКО safe-область между разделителями.
 */

ForecastScreen::ForecastScreen(
    Adafruit_ST7735& tft,
    ThemeService& theme,
    ForecastService& forecast,
    LayoutService& layout
)
: Screen(theme)
, _tft(tft)
, _forecast(forecast)
, _layout(layout)
{}

void ForecastScreen::begin() {
    _dirty = true;
    drawForecast();
}

void ForecastScreen::update() {
    if (_dirty) {
        drawForecast();
    }
}

void ForecastScreen::drawForecast() {
    _dirty = false;

    const Theme& th = theme();

    // ---------- SAFE AREA ----------
    const int y = _layout.clockY();
    const int h = _layout.clockH();

    _tft.setFont(nullptr);
    _tft.setTextWrap(false);

    // очистка области
    _tft.fillRect(0, y, _tft.width(), h, th.bg);

    // ---------- НЕТ ДАННЫХ ----------
    if (!_forecast.isReady()) {
        _tft.setTextColor(th.muted, th.bg);
        _tft.setCursor(20, y + h / 2);
        _tft.print("No forecast data");
        return;
    }

    const ForecastDay* d = _forecast.today();
    if (!d) return;

    int cy = y + 20;

    // ---------- TITLE ----------
    _tft.setTextColor(th.textSecondary, th.bg);
    _tft.setCursor((_tft.width() - 36) / 2, cy);
    _tft.print("TODAY");

    cy += 20;

    // ---------- DAY TEMP ----------
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(20, cy);
    _tft.printf("Day:   %dC", (int)round(d->tempDay));

    cy += 18;

    // ---------- NIGHT TEMP ----------
    _tft.setCursor(20, cy);
    _tft.printf("Night: %dC", (int)round(d->tempNight));

    cy += 18;

    // ---------- HUMIDITY ----------
    _tft.setTextColor(th.muted, th.bg);
    _tft.setCursor(20, cy);
    _tft.printf("Hum:   %d%%", d->humidity);
}

bool ForecastScreen::hasStatusBar() const {
    return true;
}

bool ForecastScreen::hasBottomBar() const {
    return false;
}