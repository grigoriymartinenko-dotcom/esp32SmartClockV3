#include "ui/StatusBar.h"

StatusBar::StatusBar(Adafruit_ST7735& tft)
: _tft(tft) {}

void StatusBar::setWiFi(bool connected) {
    _wifi = connected;
}

void StatusBar::setNtp(bool synced) {
    _ntp = synced;
}

void StatusBar::setNight(bool night) {
    _night = night;
}

void StatusBar::draw(int x, int y, int w, int h, bool force) {
    _x = x; _y = y; _w = w; _h = h;

    if (!force && !_first &&
        _wifi  == _lw &&
        _ntp   == _ln &&
        _night == _lni) {
        return;
    }

    _first = false;
    _lw  = _wifi;
    _ln  = _ntp;
    _lni = _night;

    drawBg();

    int cx = _x + 6;
    int cy = _y + (_h / 2);

    drawWiFi (cx, cy, _wifi);   cx += 18;
    drawNtp  (cx, cy, _ntp);    cx += 18;
    drawNight(cx, cy, _night);
}

void StatusBar::drawBg() {
    uint16_t bg = _night ? C_UI_BG_NIGHT : C_UI_BG_DAY;
    _tft.fillRect(_x, _y, _w, _h, bg);
}

void StatusBar::drawWiFi(int x, int y, bool ok) {
    uint16_t c = ok ? C_STATUS_WIFI_OK : C_STATUS_WIFI_ERR;
    _tft.drawCircle(x, y, 2, c);
    _tft.drawCircle(x, y, 4, c);
    _tft.drawCircle(x, y, 6, c);
}

void StatusBar::drawNtp(int x, int y, bool ok) {
    uint16_t c = ok ? C_STATUS_NTP_OK : C_STATUS_NTP_WAIT;
    uint16_t bg = _night ? C_UI_BG_NIGHT : C_UI_BG_DAY;

    _tft.setTextSize(1);
    _tft.setTextColor(c, bg);
    _tft.setCursor(x - 3, y - 4);
    _tft.print("N");
}

void StatusBar::drawNight(int x, int y, bool on) {
    uint16_t c = C_UI_ACCENT;

    if (on)
        _tft.fillCircle(x, y, 4, c);
    else
        _tft.drawCircle(x, y, 4, c);
}