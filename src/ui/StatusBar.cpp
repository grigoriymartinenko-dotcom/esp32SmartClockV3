#include "ui/StatusBar.h"
#include "ui/Colors.h"   // ❗ ОБЯЗАТЕЛЬНО
#include <Adafruit_GFX.h>

/*
 * StatusBar
 * ВРЕМЕННО использует C_UI_* через Colors.h (мост к themes)
 * Позже переведём на ThemeService напрямую
 */

void StatusBar::drawBg() {
    uint16_t bg = _night ? C_UI_BG_NIGHT : C_UI_BG_DAY;
    _tft.fillRect(0, 0, _w, _h, bg);
}

void StatusBar::drawWiFi(int x, int y, bool ok) {
    uint16_t c = ok ? C_STATUS_WIFI_OK : C_STATUS_WIFI_ERR;
    _tft.fillCircle(x, y, 3, c);
}

void StatusBar::drawNtp(int x, int y, bool ok) {
    uint16_t c = ok ? C_STATUS_NTP_OK : C_STATUS_NTP_WAIT;
    uint16_t bg = _night ? C_UI_BG_NIGHT : C_UI_BG_DAY;

    _tft.fillRect(x - 3, y - 3, 6, 6, bg);
    _tft.drawRect(x - 3, y - 3, 6, 6, c);
}

void StatusBar::drawNight(int x, int y, bool night) {
    uint16_t c = C_UI_ACCENT;
    if (night) {
        _tft.fillTriangle(
            x, y - 4,
            x - 3, y + 3,
            x + 3, y + 3,
            c
        );
    }
}