#include "screens/ClockScreen.h"

ClockScreen::ClockScreen(
    Adafruit_ST7735& tft,
    TimeService& time,
    NightService& night
)
: _tft(tft), _time(time), _night(night) {}

void ClockScreen::begin() {
    _lastH = _lastM = _lastS = -1;
    _colonVisible = true;
    _lastBlinkMs = millis();

    drawBackground();
    drawSeparators();
    drawTime(true);
}

void ClockScreen::update() {
    unsigned long nowMs = millis();

    // мигание двоеточия
    if (nowMs - _lastBlinkMs >= 500) {
        _lastBlinkMs = nowMs;
        _colonVisible = !_colonVisible;
        drawTime(true);
        return;
    }

    // обновление раз в секунду
    if (_time.second() != _lastS) {
        drawTime(false);
    }
}

void ClockScreen::drawBackground() {
    uint16_t bg = _night.isNight() ? C_UI_BG_NIGHT : C_UI_BG_DAY;
    _tft.fillScreen(bg);
}

void ClockScreen::drawSeparators() {
    uint16_t c = _night.isNight() ? C_GRAY_30 : C_GRAY_70;

    // линия под статусбаром
    _tft.drawFastHLine(0, SEP1_Y, _tft.width(), c);

    // линия между временем и погодой
    _tft.drawFastHLine(0, SEP2_Y, _tft.width(), c);
}

void ClockScreen::drawTime(bool force) {
    int hh = _time.hour();
    int mm = _time.minute();
    int ss = _time.second();

    if (!force && hh == _lastH && mm == _lastM && ss == _lastS) {
        return;
    }

    _lastH = hh;
    _lastM = mm;
    _lastS = ss;

    const bool night = _night.isNight();

    uint16_t bg   = night ? C_UI_BG_NIGHT : C_UI_BG_DAY;
    uint16_t main = night ? C_TIME_MAIN_NIGHT : C_TIME_MAIN_DAY;
    uint16_t secC = night ? C_TIME_SECONDS_NIGHT : C_TIME_SECONDS_DAY;

    // ---- зона времени: строго между линиями, с запасом 2px от линий ----
    const int zoneTop    = SEP1_Y + 2;
    const int zoneBottom = SEP2_Y - 2;
    const int zoneH      = zoneBottom - zoneTop;

    // чистим только внутренность зоны (линии не трогаем)
    _tft.fillRect(0, zoneTop, _tft.width(), zoneH, bg);

    // ---- формат HH:MM с мигающим двоеточием ----
    char buf[6];
    snprintf(buf, sizeof(buf), "%02d%c%02d", hh, (_colonVisible ? ':' : ' '), mm);

    _tft.setTextSize(3);
    _tft.setTextColor(main, bg);

    // важный момент: центрируем по реальному bounding box (x1,y1,w,h)
    int16_t x1, y1;
    uint16_t w, h;
    _tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);

    // top-left bounding box должен быть по центру зоны
    int topY = zoneTop + (zoneH - (int)h) / 2;
    int x = (_tft.width() - (int)w) / 2;

    // setCursor принимает baseline, поэтому компенсируем x1/y1
    _tft.setCursor(x - x1, topY - y1);
    _tft.print(buf);

    // ---- секунды: выше HH:MM (как индекс), но тоже внутри зоны ----
    char sbuf[3];
    snprintf(sbuf, sizeof(sbuf), "%02d", ss);

    _tft.setTextSize(1);
    _tft.setTextColor(secC, bg);

    int16_t sx1, sy1;
    uint16_t sw, sh;
    _tft.getTextBounds(sbuf, 0, 0, &sx1, &sy1, &sw, &sh);

    // позиция секунд: справа от HH:MM, чуть выше верхней границы HH:MM
    int secX = (x + (int)w + 6);
    if (secX + (int)sw > _tft.width() - 2) secX = _tft.width() - 2 - (int)sw;

    int secTopY = topY - (int)sh + 2;
    if (secTopY < zoneTop) secTopY = zoneTop; // чтобы не залезло в линию сверху

    _tft.setCursor(secX - sx1, secTopY - sy1);
    _tft.print(sbuf);
}