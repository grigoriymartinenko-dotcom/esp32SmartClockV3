#include "ui/BottomBar.h"
#include <math.h>

/*
 * BottomBar.cpp
 * -------------
 * Нижняя статусная строка:
 *  - вторичная информация
 *  - спокойные цвета
 *  - никакого мигания
 */

BottomBar::BottomBar(
    Adafruit_ST7735& tft,
    ThemeService& themeService,
    LayoutService& layoutService,
    DhtService& dhtService
)
: _tft(tft)
, _themeService(themeService)
, _layout(layoutService)
, _dht(dhtService)
{}

// -----------------------------------------------------
// markDirty()
// -----------------------------------------------------
void BottomBar::markDirty() {
    _dirty = true;
}

// -----------------------------------------------------
// setVisible()
// -----------------------------------------------------
void BottomBar::setVisible(bool visible) {
    if (_visible != visible) {
        _visible = visible;
        _dirty = true;
        _wasVisible = false; // ← КЛЮЧ
    }
}

// -----------------------------------------------------
// update()
// -----------------------------------------------------
void BottomBar::update() {

    // --- если панель скрыта ---
    if (!_visible) {
        if (_wasVisible) {
            clear();            // очистить один раз
            _wasVisible = false;
        }
        return;
    }

    _wasVisible = true;

    if (!_dirty)
        return;

    clear();
    drawContent();
    _dirty = false;
}

// -----------------------------------------------------
// clear()
// -----------------------------------------------------
void BottomBar::clear() {
    const Theme& theme = _themeService.current();

    _tft.fillRect(
        0,
        _layout.bottomY(),
        _tft.width(),
        _layout.bottomH(),
        theme.bg
    );
}

// -----------------------------------------------------
// drawContent()
// -----------------------------------------------------
void BottomBar::drawContent() {

    const Theme& theme = _themeService.current();

    const int y = _layout.bottomY();
    const int h = _layout.bottomH();

    _tft.setFont(nullptr);
    _tft.setTextWrap(false);
    _tft.setTextSize(1);

    // высота стандартного шрифта при size=1 ≈ 8px
    const int TEXT_H = 8;
    const int textY = y + (h - TEXT_H) / 2;

    // -------------------------------------------------
    // если данных нет
    // -------------------------------------------------
    if (!_dht.isValid()) {
        _tft.setTextColor(theme.muted, theme.bg);
        _tft.setCursor(6, textY);
        _tft.print("DHT: --°C");

        _tft.setCursor(_tft.width() - 6 - 24, textY);
        _tft.print("--%");
        return;
    }

    // -------------------------------------------------
    // данные есть
    // -------------------------------------------------
    int temp = (int)round(_dht.temperature());
    int hum  = (int)round(_dht.humidity());

    _tft.setTextColor(theme.muted, theme.bg);

    // --- температура (слева) ---
    _tft.setCursor(6, textY);
    _tft.printf("%d°C", temp);

    // --- влажность (справа) ---
    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", hum);

    // ширина строки влажности: ~6px * символ
    int humW = strlen(buf) * 6;

    _tft.setCursor(_tft.width() - 6 - humW, textY);
    _tft.print(buf);
}