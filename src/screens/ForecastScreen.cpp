#include "screens/ForecastScreen.h"
#include <math.h>
#include <stdio.h>

#include "ui/weather/WeatherIcons.h"   // ← ИКОНКИ ПОГОДЫ

/*
 * ForecastScreen.cpp
 * ------------------
 * Реактивный экран прогноза.
 *
 * КЛЮЧЕВЫЕ ПРАВИЛА:
 * 1) При ПЕРВОМ входе в READY строки ВСЕГДА чистятся
 * 2) Никаких print(int) без предварительной очистки строки
 * 3) Символ ° рисуется вручную (встроенный шрифт его не поддерживает)
 */

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
// SHORT buttons
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
// update()
// ============================================================================
void ForecastScreen::update() {

    if (themeChanged()) {
        resetCache();
        clearWorkArea();
        _dirty = true;
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

    // ================= READY =================
    const ForecastDay* d = _forecast.day(_dayIndex);
    if (!d) return;

    const int dayTemp   = (int)round(d->tempDay);
    const int nightTemp = (int)round(d->tempNight);
    const int hum       = (int)d->humidity;

    const bool force =
        _dirty ||
        stateChanged ||
        (_dayIndex != _lastDayIndex) ||
        (_lastState != UiState::READY);

    drawHeader(force, d, _dayIndex + 1, _forecast.daysCount());
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
    _lastDay = -10000;
    _lastNight = -10000;
    _lastHum = -1;
    _lastBg = theme().bg;
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
void ForecastScreen::drawHeader(bool force,
                                const ForecastDay* d,
                                uint8_t idx,
                                uint8_t total) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.contentY() + 4;

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextWrap(false);

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
// ROWS
// ============================================================================
static void drawDegreeDot(Adafruit_ST7735& tft, int x, int y, uint16_t color) {
    tft.fillCircle(x, y, 1, color);
}

// --------------------------------------------------------------------------
// DAY ROW + WEATHER ICON
// --------------------------------------------------------------------------
void ForecastScreen::drawRowDay(bool force, int t) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.contentY() + 18;

    // очищаем строку целиком
    _tft.fillRect(0, y, _tft.width(), 16, th.bg);

    // ----------------------------------------------------------------------
    // ИКОНКА ПОГОДЫ (слева)
    // ----------------------------------------------------------------------
    const ForecastDay* d = _forecast.day(_dayIndex);
    if (d) {
        WeatherIcon icon = getWeatherIcon(d->weatherCode, false);

        const int iconX = 4;
        const int iconY = y - 2;   // визуально центрируем относительно текста

        _tft.drawBitmap(
            iconX,
            iconY,
            icon.data,
            icon.width,
            icon.height,
            th.textPrimary
        );
    }

    // ----------------------------------------------------------------------
    // ТЕКСТ "Day: 25 °C"
    // ----------------------------------------------------------------------
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(32, y + 6);   // ← сдвиг вправо под иконку

    char buf[16];
    snprintf(buf, sizeof(buf), "Day:   %d", t);
    _tft.print(buf);

    int xTextEnd = 32 + strlen(buf) * 6;

    _tft.setCursor(xTextEnd + 2, y + 6);
    _tft.print(" ");

    drawDegreeDot(
        _tft,
        xTextEnd + 2 + 4,
        y + 3,
        th.textPrimary
    );

    _tft.setCursor(xTextEnd + 2 + 8, y + 6);
    _tft.print("C");
}

// --------------------------------------------------------------------------
void ForecastScreen::drawRowNight(bool force, int t) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.contentY() + 38;

    // очищаем строку целиком
    _tft.fillRect(0, y, _tft.width(), 16, th.bg);

    // ----------------------------------------------------------------------
    // ИКОНКА ПОГОДЫ (night variant)
    // ----------------------------------------------------------------------
    const ForecastDay* d = _forecast.day(_dayIndex);
    if (d) {
        // night = true → moon / night icon
        WeatherIcon icon = getWeatherIcon(d->weatherCode, true);

        const int iconX = 4;
        const int iconY = y;   // 16x16 → идеально входит в строку

        _tft.drawBitmap(
            iconX,
            iconY,
            icon.data,
            icon.width,
            icon.height,
            th.textPrimary
        );
    }

    // ----------------------------------------------------------------------
    // ТЕКСТ "Night: -3 °C"
    // ----------------------------------------------------------------------
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(32, y + 6);

    char buf[16];
    snprintf(buf, sizeof(buf), "Night: %d", t);
    _tft.print(buf);

    int xTextEnd = 32 + strlen(buf) * 6;

    _tft.setCursor(xTextEnd + 2, y + 6);
    _tft.print(" ");

    drawDegreeDot(
        _tft,
        xTextEnd + 2 + 4,
        y + 3,
        th.textPrimary
    );

    _tft.setCursor(xTextEnd + 2 + 8, y + 6);
    _tft.print("C");
}

// --------------------------------------------------------------------------
void ForecastScreen::drawRowHum(bool force, int hum) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.contentY() + 56;

    _tft.fillRect(0, y, _tft.width(), 16, th.bg);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(18, y + 6);

    char buf[16];
    snprintf(buf, sizeof(buf), "Hum:   %d%%", hum);
    _tft.print(buf);
}

// --------------------------------------------------------------------------
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
    _tft.setTextColor(th.muted, th.bg);
    _tft.print("No forecast data");
}