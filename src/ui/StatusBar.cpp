#include "ui/StatusBar.h"
#include <stdio.h>

/*
 * StatusBar.cpp
 * -------------
 * Полностью реактивная статусная панель.
 * Никаких таймеров, никакого мигания, никаких millis().
 * Рисует ТОЛЬКО когда _dirty == true.
 */

StatusBar::StatusBar(
    Adafruit_ST7735& tft,
    ThemeService& theme,
    TimeService& time
)
: _tft(tft)
, _theme(theme)
, _time(time)
{}

/*
 * markDirty()
 * -----------
 * Явно помечает StatusBar на полную перерисовку.
 */
void StatusBar::markDirty() {
    _dirty = true;
}

/*
 * setWiFiStatus()
 * ----------------
 * Вызывается СНАРУЖИ при изменении Wi-Fi состояния.
 */
void StatusBar::setWiFiStatus(Status s) {
    if (_wifi != s) {
        _wifi = s;
        markDirty();
    }
}

/*
 * setNtpStatus()
 * ---------------
 * Вызывается СНАРУЖИ при изменении NTP состояния.
 */
void StatusBar::setNtpStatus(Status s) {
    if (_ntp != s) {
        _ntp = s;
        markDirty();
    }
}

/*
 * update()
 * --------
 * Единственная точка входа для перерисовки.
 */
void StatusBar::update() {
    if (!_dirty) return;
    _dirty = false;

    draw();
}

/*
 * draw()
 * ------
 * Полная отрисовка статусбара.
 */
void StatusBar::draw() {

    const Theme& th = _theme.current();

    // --- reset GFX ---
    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextWrap(false);

    // --- фон ---
    _tft.fillRect(0, 0, _tft.width(), HEIGHT, th.bg);

    // ===== Wi-Fi (LEFT) =====
    _tft.setTextColor(statusColor(_wifi, th), th.bg);
    _tft.setCursor(4, 6);
    _tft.print('W');
    _tft.print(statusChar(_wifi));

    // ===== DATE (CENTER) =====
    if (_time.isValid()) {
        char dateBuf[12];
        snprintf(
            dateBuf,
            sizeof(dateBuf),
            "%02d.%02d.%04d",
            _time.day(),
            _time.month(),
            _time.year()
        );

        int16_t x1, y1;
        uint16_t w, h;
        _tft.getTextBounds(dateBuf, 0, 0, &x1, &y1, &w, &h);

        _tft.setTextColor(th.textSecondary, th.bg);
        _tft.setCursor((_tft.width() - w) / 2, 6);
        _tft.print(dateBuf);
    }

    // ===== NTP (RIGHT) =====
    _tft.setTextColor(statusColor(_ntp, th), th.bg);
    _tft.setCursor(_tft.width() - 24, 6);
    _tft.print('N');
    _tft.print(statusChar(_ntp));
}

/*
 * statusChar()
 * -------------
 * Символ по статусу (БЕЗ мигания).
 */
char StatusBar::statusChar(Status s) const {
    switch (s) {
        case ONLINE:     return '+';
        case CONNECTING: return '*';
        case ERROR:      return '!';
        case OFFLINE:
        default:         return '-';
    }
}

/*
 * statusColor()
 * --------------
 * Цвет по статусу.
 */
uint16_t StatusBar::statusColor(Status s, const Theme& th) const {
    switch (s) {
        case ERROR:
            return ST7735_RED;        // 🔴 явная ошибка

        case CONNECTING:
            return th.accent;         // 🔵 процесс

        case ONLINE:
            return th.textPrimary;    // 🟢 OK

        case OFFLINE:
        default:
            return th.textSecondary;  // ⚪ неактивно
    }
}