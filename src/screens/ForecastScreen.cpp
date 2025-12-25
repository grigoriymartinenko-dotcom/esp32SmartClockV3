#include "screens/ForecastScreen.h"
#include <math.h>

/*
 * ForecastScreen.cpp
 * ------------------
 * Реактивный экран прогноза:
 *  - LOADING
 *  - READY
 *  - ERROR
 *
 * Канон:
 *  - рисует ТОЛЬКО в content-зоне
 *  - не знает про BottomBar
 *  - не чистит чужие зоны
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

// ============================================================================
// begin()
// ============================================================================
void ForecastScreen::begin() {

    _tft.setFont(nullptr);
    _tft.setTextWrap(false);
    _tft.setTextSize(1);

    _dayIndex = 0;
    _lastDayIndex = 255;

    _state = UiState::LOADING;
    _lastState = UiState::ERROR;

    resetCache();
    clearWorkArea();

    _dirty = true;
}

// ============================================================================
// SHORT buttons — листание дней
// ============================================================================
void ForecastScreen::onShortLeft() {
    if (_state != UiState::READY) return;
    if (_dayIndex == 0) return;
    _dayIndex--;
    _dirty = true;
}

void ForecastScreen::onShortRight() {
    if (_state != UiState::READY) return;
    uint8_t total = _forecast.daysCount();
    if (_dayIndex + 1 >= total) return;
    _dayIndex++;
    _dirty = true;
}

// ============================================================================
// update()
// ============================================================================
void ForecastScreen::update() {

    // ---- THEME ----
    if (themeChanged()) {
        resetCache();
        clearWorkArea();
        _dirty = true;
    }

    // ---- определяем состояние ----
    if (!_forecast.isReady()) {
        _state = (_forecast.lastError()[0] == '\0')
            ? UiState::LOADING
            : UiState::ERROR;
    } else {
        _state = UiState::READY;
    }

    const bool stateChanged = (_state != _lastState);

    // ================= LOADING =================
    if (_state == UiState::LOADING) {
        drawHeader(_dirty || stateChanged, nullptr, 0, 0);
        drawLoading(_dirty || stateChanged);
        _lastState = _state;
        _dirty = false;
        return;
    }

    // ================= ERROR =================
    if (_state == UiState::ERROR) {
        drawHeader(_dirty || stateChanged, nullptr, 0, 0);
        drawError(_dirty || stateChanged);
        _lastState = _state;
        _dirty = false;
        return;
    }

    // ================= READY =================
    if (_lastState != UiState::READY) {
        clearWorkArea(); // стереть loading/error ОДИН раз
    }

    const uint8_t total = _forecast.daysCount();
    const ForecastDay* d = _forecast.day(_dayIndex);
    if (!d) return;

    const int dayTemp   = (int)round(d->tempDay);
    const int nightTemp = (int)round(d->tempNight);
    const int hum       = (int)d->humidity;

    const bool dayChanged = (_dayIndex != _lastDayIndex);
    const bool force = _dirty || stateChanged || dayChanged;

    drawHeader(force, d, _dayIndex + 1, total);
    drawRowDay(force || dayTemp != _lastDay, dayTemp);
    drawRowNight(force || nightTemp != _lastNight, nightTemp);
    drawRowHum(force || hum != _lastHum, hum);

    _lastDay = dayTemp;
    _lastNight = nightTemp;
    _lastHum = hum;
    _lastDayIndex = _dayIndex;
    _lastState = _state;

    _dirty = false;
}

// ============================================================================
// utils
// ============================================================================
void ForecastScreen::resetCache() {
    _lastDay   = -10000;
    _lastNight = -10000;
    _lastHum   = -1;
    _lastBg    = theme().bg;
}

bool ForecastScreen::themeChanged() const {
    return theme().bg != _lastBg;
}

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
// draw helpers
// ============================================================================
void ForecastScreen::drawHeader(bool force, const ForecastDay* d,
                                uint8_t idx, uint8_t total) {
    if (!force) return;

    const Theme& th = theme();
    const int baseY = _layout.contentY();

    const int rectY = baseY + 6;
    const int rectH = 18;

    _tft.fillRect(0, rectY, _tft.width(), rectH, th.bg);

    _tft.setTextSize(1);
    _tft.setTextColor(th.textSecondary, th.bg);

    const char* names[] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
    const char* dayName = d ? names[d->weekday % 7] : "---";

    _tft.setCursor(10, rectY + 5);
    _tft.print(dayName);

    _tft.setCursor(_tft.width() - 30, rectY + 5);
    if (d && total > 0) {
        _tft.printf("%d/%d", idx, total);
    } else {
        _tft.print("--/--");
    }

    bool canLeft  = (d && idx > 1);
    bool canRight = (d && idx < total);

    _tft.setTextColor(canLeft ? th.textPrimary : th.muted, th.bg);
    _tft.setCursor(2, rectY + 5);
    _tft.print("<");

    _tft.setTextColor(canRight ? th.textPrimary : th.muted, th.bg);
    _tft.setCursor(_tft.width() - 8, rectY + 5);
    _tft.print(">");
}

void ForecastScreen::drawLoading(bool force) {
    if (!force) return;

    const Theme& th = theme();
    const int baseY = _layout.contentY();

    _tft.fillRect(0, baseY + 40, _tft.width(), 24, th.bg);
    _tft.setTextColor(th.textSecondary, th.bg);
    _tft.setCursor(30, baseY + 52);
    _tft.print("Loading...");
}

void ForecastScreen::drawError(bool force) {
    if (!force) return;

    const Theme& th = theme();
    const int baseY = _layout.contentY();

    _tft.fillRect(0, baseY + 40, _tft.width(), 24, th.bg);
    _tft.setTextColor(th.muted, th.bg);
    _tft.setCursor(18, baseY + 52);
    _tft.print("No forecast data");
}

void ForecastScreen::drawRowDay(bool force, int dayTemp) {
    if (!force) return;

    const Theme& th = theme();
    const int baseY = _layout.contentY();
    const int rowY = baseY + 36;

    _tft.fillRect(0, rowY, _tft.width(), 16, th.bg);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(18, rowY + 4);
    _tft.printf("Day:   %dC", dayTemp);
}

void ForecastScreen::drawRowNight(bool force, int nightTemp) {
    if (!force) return;

    const Theme& th = theme();
    const int baseY = _layout.contentY();
    const int rowY = baseY + 54;

    _tft.fillRect(0, rowY, _tft.width(), 16, th.bg);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(18, rowY + 4);
    _tft.printf("Night: %dC", nightTemp);
}

void ForecastScreen::drawRowHum(bool force, int hum) {
    if (!force) return;

    const Theme& th = theme();
    const int baseY = _layout.contentY();
    const int rowY = baseY + 72;

    _tft.fillRect(0, rowY, _tft.width(), 16, th.bg);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(18, rowY + 4);
    _tft.printf("Hum:   %d%%", hum);
}