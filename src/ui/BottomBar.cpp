#include "ui/BottomBar.h"
#include <math.h>

/*
 * BottomBar.cpp
 * -------------
 * Полностью реактивный нижний бар.
 * Никаких таймеров, никаких авто-перерисовок.
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

/*
 * markDirty()
 * -----------
 * Вызывается при изменении данных (DHT / тема).
 */
void BottomBar::markDirty() {
    _dirty = true;
}

/*
 * setVisible()
 * ------------
 * ScreenManager сообщает, нужен ли BottomBar.
 */
void BottomBar::setVisible(bool visible) {
    if (_visible != visible) {
        _visible = visible;
        _dirty = true;
    }
}

/*
 * update()
 * --------
 * Единственная точка обновления.
 */
void BottomBar::update() {

    // --- скрытие ---
    if (!_visible) {
        if (_wasVisible) {
            clear();          // стираем ОДИН раз
            _wasVisible = false;
        }
        return;
    }

    _wasVisible = true;

    if (!_dirty) return;

    clear();
    drawContent();
    _dirty = false;
}

/*
 * clear()
 * -------
 * Очистка ТОЛЬКО области BottomBar.
 */
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

/*
 * drawContent()
 * -------------
 * Рисует содержимое BottomBar.
 */
void BottomBar::drawContent() {

    const Theme& theme = _themeService.current();

    const int y = _layout.bottomY();
    const int h = _layout.bottomH();

    // --- reset GFX ---
    _tft.setFont(nullptr);
    _tft.setTextWrap(false);

    // --- нет данных ---
    if (!_dht.isValid()) {
        _tft.setTextSize(1);
        _tft.setTextColor(theme.muted, theme.bg);
        _tft.setCursor(6, y + (h / 2) - 4);
        _tft.print("DHT: --C  --%");
        return;
    }

    // --- данные есть ---
    int t   = (int)round(_dht.temperature());
    int hum = (int)round(_dht.humidity());

    _tft.setTextSize(2);
    _tft.setTextColor(theme.textPrimary, theme.bg);

    // textSize(2) ≈ 16px высоты
    const int textY = y + (h - 16) / 2;
    _tft.setCursor(6, textY);
    _tft.printf("%dC  %d%%", t, hum);
}