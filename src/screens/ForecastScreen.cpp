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
 * (ThemeService + NightTransitionService + ColorTemperatureService).
 *
 * UX:
 *  - анимация перелистывания дней: slide + лёгкий "fade" (ступенчато через цвета)
 */

// ============================================================================
// helpers
// ============================================================================

static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

// Простейшее smoothstep для прогресса анимации (0..1).
static float smooth01(float t) {
    t = clamp01(t);
    return t * t * (3.0f - 2.0f * t);
}

// "Псевдо-fade" без альфы: на первых кадрах рисуем muted,
// ближе к финалу — fg. Это не настоящая прозрачность, но выглядит мягче.
static uint16_t fadeTextColor(const ThemeBlend& b, float k) {
    // k: 0..1 (0 - приглушённо, 1 - ярко)
    // Мы не умеем смешивать 565 корректно без утилиты, поэтому:
    // делаем дискрет: 0..0.45 -> muted, 0.45..0.75 -> fg, >0.75 -> fg
    // (да, два шага. На TFT это выглядит ок и дешево.)
    if (k < 0.45f) return b.muted;
    return b.fg;
}

static void drawDegreeDot(
    Adafruit_ST7735& tft,
    int x,
    int y,
    uint16_t color
) {
    tft.fillCircle(x, y, 1, color);
}

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

    _animActive  = false;
    _animStartMs = 0;
    _animDir     = 0;

    _dirty = true;
}

// ============================================================================
// buttons
// ============================================================================
void ForecastScreen::onShortLeft() {
    if (_state != UiState::READY) return;
    if (_animActive) return;
    if (_dayIndex == 0) return;

    startDayTransition(-1);
}

void ForecastScreen::onShortRight() {
    if (_state != UiState::READY) return;
    if (_animActive) return;
    if (_dayIndex + 1 >= _forecast.daysCount()) return;

    startDayTransition(+1);
}

// ============================================================================
// animation control
// ============================================================================
void ForecastScreen::startDayTransition(int dir) {
    if (dir != -1 && dir != +1) return;

    const int next = (int)_dayIndex + dir;
    if (next < 0) return;
    if (next >= (int)_forecast.daysCount()) return;

    _animActive  = true;
    _animStartMs = millis();
    _animDir     = dir;
    _animFrom    = _dayIndex;
    _animTo      = (uint8_t)next;

    // Во время анимации мы будем рисовать оба дня по кадрам.
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

    // Если анимация активна — мы рисуем кадры, даже если "вроде ничего не менялось".
    if (_animActive) {
        redrawAll();
        return;
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

    // ЕДИНСТВЕННАЯ точка получения цветов (уже после Day/Night + ColorTemp)
    const ThemeBlend& b = themeService().blend();

    // ------------------------------------------------------------------------
    // Фон рабочей области (чистим всегда, потому что:
    //  - анимация требует чистого холста
    //  - и это гарантирует отсутствие "хвостов"
    // ------------------------------------------------------------------------
    _tft.fillRect(
        0,
        _layout.contentY(),
        _tft.width(),
        _layout.contentH(),
        b.bg
    );

    // ----- LOADING -----
    if (_state == UiState::LOADING) {
        drawHeaderAtX(b, nullptr, 0, 0, 0);
        drawLoading(b);
        _animActive = false;
        _dirty = false;
        _lastState = _state;
        return;
    }

    // ----- ERROR -----
    if (_state == UiState::ERROR) {
        drawHeaderAtX(b, nullptr, 0, 0, 0);
        drawError(b);
        _animActive = false;
        _dirty = false;
        _lastState = _state;
        return;
    }

    // ----- READY -----
    // Если идёт анимация — рисуем кадр, иначе рисуем обычный экран.
    if (_animActive) {
        drawTransitionFrame(b);
        return;
    }

    const ForecastDay* d = _forecast.day(_dayIndex);
    if (!d) return;

    drawReadyAtX(b, d, _dayIndex + 1, _forecast.daysCount(), 0);

    _lastState    = _state;
    _lastDayIndex = _dayIndex;
    _dirty        = false;
}

// ============================================================================
// transition frame
// ============================================================================
void ForecastScreen::drawTransitionFrame(const ThemeBlend& b) {

    const uint32_t now = millis();
    uint32_t elapsed = now - _animStartMs;
    if (elapsed > ANIM_MS) elapsed = ANIM_MS;

    float t = (float)elapsed / (float)ANIM_MS;     // 0..1
    t = smooth01(t);

    const int W = _tft.width();

    // Старый уезжает в сторону dir (если dir=+1, старый уезжает влево),
    // новый заезжает с противоположной стороны.
    //
    // Пример dir=+1 (лист вправо, следующий день):
    //  old: x = -t*W
    //  new: x = (1-t)*W
    //
    // Пример dir=-1 (лист влево, предыдущий день):
    //  old: x = +t*W
    //  new: x = -(1-t)*W
    //
    const int xOld = (int)lroundf((float)(- _animDir) * t * (float)W);
    const int xNew = (int)lroundf((float)(- _animDir) * (t - 1.0f) * (float)W);

    const ForecastDay* dOld = _forecast.day(_animFrom);
    const ForecastDay* dNew = _forecast.day(_animTo);

    // Если вдруг данных нет — прекращаем анимацию.
    if (!dOld || !dNew) {
        _animActive = false;
        _dayIndex = _animTo;
        _dirty = true;
        return;
    }

    // Рисуем оба дня. Порядок: сначала old, потом new (чтобы new был "сверху").
    drawReadyAtX(b, dOld, _animFrom + 1, _forecast.daysCount(), xOld);
    drawReadyAtX(b, dNew, _animTo   + 1, _forecast.daysCount(), xNew);

    // Завершение
    if (elapsed >= ANIM_MS) {
        _animActive  = false;
        _dayIndex    = _animTo;
        _lastDayIndex = 255; // форсим корректный redraw при следующем апдейте
        _dirty       = true;
    }
}

// ============================================================================
// ready (one day) at x offset
// ============================================================================
void ForecastScreen::drawReadyAtX(
    const ThemeBlend& b,
    const ForecastDay* d,
    uint8_t idx,
    uint8_t total,
    int xOff
) {
    drawHeaderAtX(b, d, idx, total, xOff);
    drawRowDayAtX(b, d, xOff);
    drawRowNightAtX(b, d, xOff);
    drawRowHumAtX(b, d->humidity, xOff);
}

// ============================================================================
// header at x offset
// ============================================================================
void ForecastScreen::drawHeaderAtX(
    const ThemeBlend& b,
    const ForecastDay* d,
    uint8_t idx,
    uint8_t total,
    int xOff
) {
    const int y = _layout.contentY() + 4;

    // Полоса заголовка — внутри content области, поэтому можно рисовать с оффсетом.
    // Фон заголовка уже очищен в redrawAll(), но для безопасности подчистим.
    _tft.fillRect(xOff, y, _tft.width(), 18, b.bg);

    const char* names[] = {
        "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
    };

    _tft.setTextColor(b.muted, b.bg);

    _tft.setCursor(xOff + 10, y + 4);
    _tft.print(d ? names[d->weekday % 7] : "---");

    _tft.setCursor(xOff + _tft.width() - 30, y + 4);
    if (d && total) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d/%d", idx, total);
        _tft.print(buf);
    } else {
        _tft.print("--/--");
    }
}

// ============================================================================
// rows at x offset
// ============================================================================
void ForecastScreen::drawRowDayAtX(
    const ThemeBlend& b,
    const ForecastDay* d,
    int xOff
) {
    const int y = _layout.contentY() + 18;
    _tft.fillRect(xOff, y, _tft.width(), 16, b.bg);

    // Иконка дня
    WeatherIcon icon = getWeatherIcon(d->weatherCode, false);

    // ВАЖНО:
    // Экран не решает "ночь/день". Цвет иконки берём из ThemeBlend (уже
    // обработанного ColorTemperatureService'ом).
    const uint16_t iconColor = b.fg;

    _tft.drawBitmap(
        xOff + 4, y + 1,
        icon.data,
        icon.width,
        icon.height,
        iconColor
    );

    // Псевдо-fade для текста (в анимации выглядит мягче):
    // вычисляем "видимую яркость" по положению на экране
    float k = 1.0f;
    // если уезжаем далеко — немного приглушаем
    if (xOff != 0) {
        k = 1.0f - (fabsf((float)xOff) / (float)_tft.width());
        k = clamp01(k);
    }

    const uint16_t tc = fadeTextColor(b, k);

    _tft.setTextColor(tc, b.bg);
    _tft.setCursor(xOff + 32, y + 6);

    float temp =
        !isnan(d->tempDay)   ? d->tempDay :
        !isnan(d->tempNight) ? d->tempNight :
                               NAN;

    if (isnan(temp)) {
        _tft.print("Day:   --");
        return;
    }

    int tInt = (int)lround(temp);
    char buf[16];
    snprintf(buf, sizeof(buf), "Day:   %d", tInt);
    _tft.print(buf);

    int x = xOff + 32 + (int)strlen(buf) * 6;

    drawDegreeDot(_tft, x + 6, y + 4, tc);
    _tft.setCursor(x + 10, y + 6);
    _tft.print("C");
}

void ForecastScreen::drawRowNightAtX(
    const ThemeBlend& b,
    const ForecastDay* d,
    int xOff
) {
    const int y = _layout.contentY() + 38;
    _tft.fillRect(xOff, y, _tft.width(), 16, b.bg);

    // Иконка ночи
    WeatherIcon icon = getWeatherIcon(d->weatherCode, true);
    const uint16_t iconColor = b.fg;

    _tft.drawBitmap(
        xOff + 4, y + 1,
        icon.data,
        icon.width,
        icon.height,
        iconColor
    );

    float k = 1.0f;
    if (xOff != 0) {
        k = 1.0f - (fabsf((float)xOff) / (float)_tft.width());
        k = clamp01(k);
    }
    const uint16_t tc = fadeTextColor(b, k);

    _tft.setTextColor(tc, b.bg);
    _tft.setCursor(xOff + 32, y + 6);

    if (isnan(d->tempNight)) {
        _tft.print("Night: --");
        return;
    }

    int tInt = (int)lround(d->tempNight);
    char buf[16];
    snprintf(buf, sizeof(buf), "Night: %d", tInt);
    _tft.print(buf);

    int x = xOff + 32 + (int)strlen(buf) * 6;

    drawDegreeDot(_tft, x + 6, y + 4, tc);
    _tft.setCursor(x + 10, y + 6);
    _tft.print("C");
}

void ForecastScreen::drawRowHumAtX(
    const ThemeBlend& b,
    int hum,
    int xOff
) {
    const int y = _layout.contentY() + 56;
    _tft.fillRect(xOff, y, _tft.width(), 16, b.bg);

    float k = 1.0f;
    if (xOff != 0) {
        k = 1.0f - (fabsf((float)xOff) / (float)_tft.width());
        k = clamp01(k);
    }
    const uint16_t tc = (k < 0.55f) ? b.muted : b.muted;

    _tft.setTextColor(tc, b.bg);
    _tft.setCursor(xOff + 32, y + 6);

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