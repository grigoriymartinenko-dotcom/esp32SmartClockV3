#include "screens/ForecastScreen.h"
#include <math.h>

/*
 * ForecastScreen
 * --------------
 * Реактивный экран прогноза (today only).
 *
 * Главная цель:
 *  - НЕТ полного fillRect каждый update()
 *  - частичная перерисовка строк по изменению данных
 *
 * ВАЖНО:
 *  - begin(): очищаем рабочую область ОДИН раз
 *  - update(): сравниваем данные с кешем и перерисовываем только изменившееся
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

/*
 * begin()
 * -------
 * Экран стал активным:
 *  - чистим свою область ОДИН раз
 *  - сбрасываем кеш
 *  - помечаем на первичную отрисовку
 */
void ForecastScreen::begin() {

    // reset GFX (Adafruit любит "наследовать" настройки)
    _tft.setFont(nullptr);
    _tft.setTextWrap(false);
    _tft.setTextSize(1);

    resetCache();

    // полный фон только при входе на экран
    clearWorkArea();
    hardClearBottom2px();

    _dirty = true;
}

/*
 * update()
 * --------
 * Частичная перерисовка: сравниваем текущие данные и кеш.
 * Никаких таймеров / millis.
 */
void ForecastScreen::update() {

    // если тема сменилась (ночь/день) — делаем полный redraw один раз
    if (themeChanged()) {
        resetCache();
        clearWorkArea();
        hardClearBottom2px();
        _dirty = true;
    }

    const bool ready = _forecast.isReady();
    const ForecastDay* d = ready ? _forecast.today() : nullptr;

    // если данных нет — рисуем заглушку, но только если состояние изменилось
    if (!ready || !d) {
        drawTitle(_dirty);          // заголовок можно оставить
        drawNoData(_dirty || _lastReady != false);

        _lastReady = false;
        _dirty = false;
        return;
    }

    // данные есть — получаем округлённые значения
    const int dayTemp   = (int)round(d->tempDay);
    const int nightTemp = (int)round(d->tempNight);
    const int hum       = (int)round(d->humidity);

    // первичная отрисовка или частичные изменения
    drawTitle(_dirty);

    // если до этого была заглушка "No data" — нужно затереть её область один раз
    if (_lastReady == false && !_dirty) {
        // просто принудительно перерисуем все строки один раз
        drawRowDay(true, dayTemp);
        drawRowNight(true, nightTemp);
        drawRowHum(true, hum);
    } else {
        drawRowDay(_dirty || dayTemp != _lastDay, dayTemp);
        drawRowNight(_dirty || nightTemp != _lastNight, nightTemp);
        drawRowHum(_dirty || hum != _lastHum, hum);
    }

    // обновляем кеш
    _lastReady = true;
    _lastDay   = dayTemp;
    _lastNight = nightTemp;
    _lastHum   = hum;

    // низ иногда “призрачит” на ST7735 — чистим только 2 строки (дёшево)
    hardClearBottom2px();

    _dirty = false;
}

/*
 * ===== УТИЛИТЫ =====
 */

void ForecastScreen::resetCache() {
    _lastReady = false;
    _lastDay   = -10000;
    _lastNight = -10000;
    _lastHum   = -1;

    _lastBg = theme().bg;
}

bool ForecastScreen::themeChanged() const {
    return theme().bg != _lastBg;
}

void ForecastScreen::clearWorkArea() {
    const Theme& th = theme();

    const int y = _layout.clockY();     // сразу под StatusBar
    const int h = _tft.height() - y;    // до низа дисплея

    _tft.fillRect(0, y, _tft.width(), h, th.bg);
}

void ForecastScreen::hardClearBottom2px() {
    const Theme& th = theme();

    _tft.fillRect(
        0,
        _tft.height() - 2,
        _tft.width(),
        2,
        th.bg
    );
}

/*
 * ===== РИСОВАНИЕ ЭЛЕМЕНТОВ (ЧАСТИЧНО) =====
 *
 * Мы намеренно рисуем в фиксированных прямоугольниках:
 * - чтобы текст не "дёргался"
 * - чтобы можно было чистить только изменившуюся строку
 */

void ForecastScreen::drawTitle(bool force) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.clockY();

    // фиксированная зона заголовка
    const int rectY = y + 8;
    const int rectH = 16;

    _tft.fillRect(0, rectY, _tft.width(), rectH, th.bg);

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextColor(th.textSecondary, th.bg);

    // центрируем "TODAY" как и было, но без getTextBounds (фикс. ширина 36)
    _tft.setCursor((_tft.width() - 36) / 2, rectY + 4);
    _tft.print("TODAY");
}

void ForecastScreen::drawNoData(bool force) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.clockY();
    const int h = _tft.height() - y;

    // зона под заглушку — середина экрана
    const int rectY = y + h / 2 - 10;
    const int rectH = 20;

    _tft.fillRect(0, rectY, _tft.width(), rectH, th.bg);

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextColor(th.muted, th.bg);

    _tft.setCursor(20, rectY + 8);
    _tft.print("No forecast data");
}

void ForecastScreen::drawRowDay(bool force, int dayTemp) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.clockY();

    // как раньше: после заголовка отступы похожие
    const int rowY = y + 14 + 22;   // y + 36
    const int rowH = 16;

    _tft.fillRect(0, rowY, _tft.width(), rowH, th.bg);

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextColor(th.textPrimary, th.bg);

    _tft.setCursor(20, rowY + 4);
    _tft.printf("Day:   %dC", dayTemp);
}

void ForecastScreen::drawRowNight(bool force, int nightTemp) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.clockY();

    const int rowY = y + 14 + 22 + 18; // следующая строка
    const int rowH = 16;

    _tft.fillRect(0, rowY, _tft.width(), rowH, th.bg);

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextColor(th.textPrimary, th.bg);

    _tft.setCursor(20, rowY + 4);
    _tft.printf("Night: %dC", nightTemp);
}

void ForecastScreen::drawRowHum(bool force, int hum) {
    if (!force) return;

    const Theme& th = theme();
    const int y = _layout.clockY();

    const int rowY = y + 14 + 22 + 18 + 18; // третья строка
    const int rowH = 16;

    _tft.fillRect(0, rowY, _tft.width(), rowH, th.bg);

    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextColor(th.textPrimary, th.bg);

    _tft.setCursor(20, rowY + 4);
    _tft.printf("Hum:   %d%%", hum);
}