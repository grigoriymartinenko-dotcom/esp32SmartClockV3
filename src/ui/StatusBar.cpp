#include "ui/StatusBar.h"
#include <stdio.h>

/*
 * Вспомогательные функции для отображения статусов
 */

// символ по статусу
static char statusChar(StatusBar::Status s, bool blinkOn) {
    switch (s) {
        case StatusBar::ONLINE:     return '+';
        case StatusBar::CONNECTING: return blinkOn ? '*' : ' ';
        case StatusBar::ERROR:      return '!';
        default:                    return '-';
    }
}

// цвет по статусу
static uint16_t statusColor(StatusBar::Status s, const Theme& th) {
    switch (s) {
        case StatusBar::ERROR:
            return ST7735_RED;      // 🔴 явная ошибка

        case StatusBar::CONNECTING:
            return th.accent;       // 🔵 процесс (мигает)

        case StatusBar::ONLINE:
            return th.textPrimary;  // 🟢 OK = ЗЕЛЁНЫЙ

        case StatusBar::OFFLINE:
        default:
            return ST7735_GREEN;        // ⚪ неактивно
    }
}

StatusBar::StatusBar(
    Adafruit_ST7735& tft,
    ThemeService& theme,
    TimeService& time
)
: _tft(tft), _theme(theme), _time(time)
{}

void StatusBar::markDirty() {
    _dirty = true;
}

void StatusBar::setWiFiStatus(Status s) {
    if (_wifi != s) {
        _wifi = s;
        markDirty();
    }
}

void StatusBar::setNtpStatus(Status s) {
    if (_ntp != s) {
        _ntp = s;
        markDirty();
    }
}

void StatusBar::draw() {
    // мигание: 500 мс
    static uint32_t lastBlinkMs = 0;
    static bool blinkOn = true;

    if (millis() - lastBlinkMs > 500) {
        lastBlinkMs = millis();
        blinkOn = !blinkOn;
        if (_wifi == CONNECTING || _ntp == CONNECTING)
            _dirty = true; // перерисовываем ТОЛЬКО если мигаем
    }

    if (!_dirty) return;
    _dirty = false;

    const Theme& th = _theme.current();

    // reset GFX
    _tft.setFont(nullptr);
    _tft.setTextSize(1);
    _tft.setTextWrap(false);

    // фон
    _tft.fillRect(0, 0, _tft.width(), HEIGHT, th.bg);

    // ===== Wi-Fi (LEFT) =====
    _tft.setTextColor(statusColor(_wifi, th), th.bg);
    _tft.setCursor(4, 6);
    _tft.print('W');
    _tft.print(statusChar(_wifi, blinkOn));

    // ===== DATE (CENTER) =====
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

    // ===== NTP (RIGHT) =====
    _tft.setTextColor(statusColor(_ntp, th), th.bg);
    _tft.setCursor(_tft.width() - 24, 6);
    _tft.print('N');
    _tft.print(statusChar(_ntp, blinkOn));
}