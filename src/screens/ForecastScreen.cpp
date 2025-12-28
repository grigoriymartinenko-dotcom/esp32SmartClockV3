#include "screens/ForecastScreen.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// ctor
// ============================================================================
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

// ============================================================================
// begin
// ============================================================================
void ForecastScreen::begin() {
    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextWrap(false);

    _dayIndex = 0;
    _lastDayIndex = 255;

    _state = UiState::LOADING;
    _lastState = UiState::ERROR;

    clearWorkArea();
    _dirty = true;
}

// ============================================================================
// buttons
// ============================================================================
void ForecastScreen::onShortLeft() {
    if (_state != UiState::READY || _dayIndex == 0) return;
    _dayIndex--;
    _dirty = true;
}

void ForecastScreen::onShortRight() {
    if (_state != UiState::READY) return;
    if (_dayIndex + 1 >= _forecast.daysCount()) return;
    _dayIndex++;
    _dirty = true;
}

// ============================================================================
// update
// ============================================================================
void ForecastScreen::update() {

    if (_forecast.isUpdating()) {
        return;
    }

    if (!_forecast.isReady()) {
        _state = (_forecast.lastError()[0] == '\0')
            ? UiState::LOADING
            : UiState::ERROR;
    } else {
        _state = UiState::READY;
    }

    const bool stateChanged = (_state != _lastState);

    if (_state == UiState::LOADING) {
        drawHeader(_dirty || stateChanged, nullptr, 0, 0);
        drawLoading(_dirty || stateChanged);
        _lastState = _state;
        _dirty = false;
        return;
    }

    if (_state == UiState::ERROR) {
        drawHeader(_dirty || stateChanged, nullptr, 0, 0);
        drawError(_dirty || stateChanged);
        _lastState = _state;
        _dirty = false;
        return;
    }

    // READY
    const ForecastDay* d = _forecast.day(_dayIndex);
    if (!d) return;

    const bool force =
        _dirty ||
        stateChanged ||
        (_dayIndex != _lastDayIndex);

    drawHeader(force, d, _dayIndex + 1, _forecast.daysCount());
    drawRowDay(force, d);
    drawRowNight(force, d);
    drawRowHum(force, d->humidity);

    _lastDayIndex = _dayIndex;
    _lastState = _state;
    _dirty = false;
}

// ============================================================================
// utils
// ============================================================================
void ForecastScreen::clearWorkArea() {
    const Theme& th = theme();
    _tft.fillRect(
        0,
        _layout.contentY(),
        _tft.width(),
        _layout.contentH(),
        th.bg
    );
}

// ============================================================================
// header
// ============================================================================
void ForecastScreen::drawHeader(bool force,
                                const ForecastDay* d,
                                uint8_t idx,
                                uint8_t total) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.contentY() + 4;

    _tft.fillRect(0, y, _tft.width(), 18, th.bg);
    _tft.setTextColor(th.textSecondary, th.bg);

    const char* names[] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
    _tft.setCursor(10, y + 4);
    _tft.print(d ? names[d->weekday % 7] : "---");

    _tft.setCursor(_tft.width() - 30, y + 4);
    if (d && total) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d/%d", idx, total);
        _tft.print(buf);
    } else {
        _tft.print("--/--");
    }
}

// ============================================================================
// rows
// ============================================================================
static void drawDegreeDot(Adafruit_ST7735& tft, int x, int y, uint16_t c) {
    tft.fillCircle(x, y, 1, c);
}

// ---------------------------------------------------------------------------
// DAY ROW
// ---------------------------------------------------------------------------
void ForecastScreen::drawRowDay(bool force, const ForecastDay* d) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.contentY() + 18;

    _tft.fillRect(0, y, _tft.width(), 16, th.bg);

    // ИКОНКА — реально опущена
    WeatherIcon icon = getWeatherIcon(d->weatherCode, false);
    _tft.drawBitmap(4, y + 1, icon.data, icon.width, icon.height, th.textPrimary);

    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(32, y + 6);

    float temp =
        !isnan(d->tempDay)   ? d->tempDay :
        !isnan(d->tempNight) ? d->tempNight :
                               NAN;

    if (isnan(temp)) {
        _tft.print("Day:   --");
        return;
    }

    int t = (int)lround(temp);
    char buf[16];
    snprintf(buf, sizeof(buf), "Day:   %d", t);
    _tft.print(buf);

    int x = 32 + strlen(buf) * 6;
    drawDegreeDot(_tft, x + 6, y + 4, th.textPrimary);
    _tft.setCursor(x + 10, y + 6);
    _tft.print("C");
}

// ---------------------------------------------------------------------------
// NIGHT ROW
// ---------------------------------------------------------------------------
void ForecastScreen::drawRowNight(bool force, const ForecastDay* d) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.contentY() + 38;

    _tft.fillRect(0, y, _tft.width(), 16, th.bg);

    WeatherIcon icon = getWeatherIcon(d->weatherCode, true);
    _tft.drawBitmap(4, y + 1, icon.data, icon.width, icon.height, th.textPrimary);

    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(32, y + 6);

    if (isnan(d->tempNight)) {
        _tft.print("Night: --");
        return;
    }

    int t = (int)lround(d->tempNight);
    char buf[16];
    snprintf(buf, sizeof(buf), "Night: %d", t);
    _tft.print(buf);

    int x = 32 + strlen(buf) * 6;
    drawDegreeDot(_tft, x + 6, y + 4, th.textPrimary);
    _tft.setCursor(x + 10, y + 6);
    _tft.print("C");
}

// ---------------------------------------------------------------------------
// HUMIDITY
// ---------------------------------------------------------------------------
void ForecastScreen::drawRowHum(bool force, int hum) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.contentY() + 56;

    _tft.fillRect(0, y, _tft.width(), 16, th.bg);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(32, y + 6);

    char buf[16];
    snprintf(buf, sizeof(buf), "Hum:   %d%%", hum);
    _tft.print(buf);
}

// ============================================================================
// states
// ============================================================================
void ForecastScreen::drawLoading(bool force) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.contentY() + 36;

    _tft.fillRect(0, y, _tft.width(), 20, th.bg);
    _tft.setCursor(30, y);
    _tft.setTextColor(th.textSecondary, th.bg);
    _tft.print("Loading...");
}

void ForecastScreen::drawError(bool force) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.contentY() + 36;

    _tft.fillRect(0, y, _tft.width(), 20, th.bg);
    _tft.setCursor(18, y + 10);
    _tft.setTextColor(th.warn, th.bg);
    _tft.print("No forecast data");
}