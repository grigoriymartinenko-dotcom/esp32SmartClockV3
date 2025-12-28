#include "screens/ForecastScreen.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

/*
 * ForecastScreen.cpp
 * ------------------
 * Экран прогноза погоды.
 *
 * ПРИНЦИПЫ:
 *  - Экран не знает Day / Night
 *  - Экран использует ТОЛЬКО ThemeBlend
 *  - Экран реактивен через UiVersionService
 *
 * Любые изменения цвета / темы приходят извне
 * (ThemeService + NightTransitionService).
 */

// ============================================================================
// ctor
// ============================================================================
ForecastScreen::ForecastScreen(
    Adafruit_ST7735&  tft,
    ThemeService&     theme,
    ForecastService&  forecast,
    LayoutService&    layout,
    UiVersionService& ui
)
    : Screen(theme)
    , _tft(tft)
    , _forecast(forecast)
    , _layout(layout)
    , _ui(ui)
{
}

// ============================================================================
// begin
// ============================================================================
void ForecastScreen::begin() {

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextWrap(false);

    _dayIndex     = 0;
    _lastDayIndex = 255;

    _state     = UiState::LOADING;
    _lastState = UiState::ERROR;

    _dirty = true;
}

// ============================================================================
// buttons
// ============================================================================
void ForecastScreen::onShortLeft() {
    if (_state != UiState::READY) return;
    if (_dayIndex == 0) return;

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
// update (reactive)
// ============================================================================
void ForecastScreen::update() {

    // Реакция на смену темы или явный запрос перерисовки экрана
    if (_ui.changed(UiChannel::THEME) ||
        _ui.changed(UiChannel::SCREEN)) {
        _dirty = true;
    }

    // Пока сервис обновляется — UI не трогаем
    if (_forecast.isUpdating()) return;

    // Определяем состояние
    if (!_forecast.isReady()) {
        _state = (_forecast.lastError()[0] == '\0')
            ? UiState::LOADING
            : UiState::ERROR;
    } else {
        _state = UiState::READY;
    }

    // Ничего не изменилось — выходим
    if (!_dirty &&
        _state == _lastState &&
        _dayIndex == _lastDayIndex) {
        return;
    }

    redrawAll();

    _lastState    = _state;
    _lastDayIndex = _dayIndex;
    _dirty        = false;
}

// ============================================================================
// redraw all
// ============================================================================
void ForecastScreen::redrawAll() {

    // ЕДИНСТВЕННАЯ точка получения цветов
    const ThemeBlend& b = themeService().blend();

    // Фон рабочей области
    _tft.fillRect(
        0,
        _layout.contentY(),
        _tft.width(),
        _layout.contentH(),
        b.bg
    );

    // ----- LOADING -----
    if (_state == UiState::LOADING) {
        drawHeader(b, nullptr, 0, 0);
        drawLoading(b);
        return;
    }

    // ----- ERROR -----
    if (_state == UiState::ERROR) {
        drawHeader(b, nullptr, 0, 0);
        drawError(b);
        return;
    }

    // ----- READY -----
    const ForecastDay* d = _forecast.day(_dayIndex);
    if (!d) return;

    drawHeader(b, d, _dayIndex + 1, _forecast.daysCount());
    drawRowDay(b, d);
    drawRowNight(b, d);
    drawRowHum(b, d->humidity);
}

// ============================================================================
// header
// ============================================================================
void ForecastScreen::drawHeader(
    const ThemeBlend& b,
    const ForecastDay* d,
    uint8_t idx,
    uint8_t total
) {
    const int y = _layout.contentY() + 4;

    _tft.fillRect(0, y, _tft.width(), 18, b.bg);
    _tft.setTextColor(b.muted, b.bg);

    const char* names[] = {
        "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
    };

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
// helpers
// ============================================================================
static void drawDegreeDot(
    Adafruit_ST7735& tft,
    int x,
    int y,
    uint16_t color
) {
    tft.fillCircle(x, y, 1, color);
}

// ============================================================================
// rows
// ============================================================================
void ForecastScreen::drawRowDay(
    const ThemeBlend& b,
    const ForecastDay* d
) {
    const int y = _layout.contentY() + 18;
    _tft.fillRect(0, y, _tft.width(), 16, b.bg);

    // Иконка дня
    WeatherIcon icon = getWeatherIcon(d->weatherCode, false);
    const uint16_t iconColor =
    themeService().isNight() ? b.fgWarm : b.fgCool;

_tft.drawBitmap(
    4, y + 1,
    icon.data,
    icon.width,
    icon.height,
    iconColor
);

    _tft.setTextColor(b.fg, b.bg);
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
    drawDegreeDot(_tft, x + 6, y + 4, b.fg);
    _tft.setCursor(x + 10, y + 6);
    _tft.print("C");
}

void ForecastScreen::drawRowNight(
    const ThemeBlend& b,
    const ForecastDay* d
) {
    const int y = _layout.contentY() + 38;
    _tft.fillRect(0, y, _tft.width(), 16, b.bg);

    // Иконка ночи
    WeatherIcon icon = getWeatherIcon(d->weatherCode, true);
const uint16_t iconColor =
    themeService().isNight() ? b.fgWarm : b.fgCool;

_tft.drawBitmap(
    4, y + 1,
    icon.data,
    icon.width,
    icon.height,
    iconColor
);

    _tft.setTextColor(b.fg, b.bg);
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
    drawDegreeDot(_tft, x + 6, y + 4, b.fg);
    _tft.setCursor(x + 10, y + 6);
    _tft.print("C");
}

void ForecastScreen::drawRowHum(
    const ThemeBlend& b,
    int hum
) {
    const int y = _layout.contentY() + 56;
    _tft.fillRect(0, y, _tft.width(), 16, b.bg);

    _tft.setTextColor(b.muted, b.bg);
    _tft.setCursor(32, y + 6);

    char buf[16];
    snprintf(buf, sizeof(buf), "Hum:   %d%%", hum);
    _tft.print(buf);
}

// ============================================================================
// states
// ============================================================================
void ForecastScreen::drawLoading(const ThemeBlend& b) {

    const int y = _layout.contentY() + 36;
    _tft.fillRect(0, y, _tft.width(), 20, b.bg);

    _tft.setCursor(30, y + 4);
    _tft.setTextColor(b.muted, b.bg);
    _tft.print("Loading...");
}

void ForecastScreen::drawError(const ThemeBlend& b) {

    const int y = _layout.contentY() + 36;
    _tft.fillRect(0, y, _tft.width(), 20, b.bg);

    _tft.setCursor(18, y + 6);
    _tft.setTextColor(b.warn, b.bg);
    _tft.print("No forecast data");
}