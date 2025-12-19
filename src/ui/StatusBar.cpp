#include "ui/StatusBar.h"
#include <Fonts/FreeSans9pt7b.h>

/*
 * ------------------------------------------------------------
 * Конструктор
 * ------------------------------------------------------------
 */
StatusBar::StatusBar(
    Adafruit_ST7735& tft,
    ThemeService& theme,
    TimeService& time
)
: _tft(tft),
  _theme(theme),
  _time(time)
{}

/*
 * Установка состояния Wi-Fi
 */
void StatusBar::setWiFi(bool connected) {
    _wifiOk = connected;
}

/*
 * Проверка, нужно ли перерисовываться
 */
bool StatusBar::isDirty() const {
    if (_wifiOk          != _lastWifiOk)  return true;
    if (_time.isValid()  != _lastTimeOk)  return true;
    if (_theme.isNight() != _lastIsNight) return true;

    if (_time.isValid()) {
        if (_time.day()   != _lastDay)   return true;
        if (_time.month() != _lastMonth) return true;
        if (_time.year()  != _lastYear)  return true;
    }

    return false;
}

/*
 * ------------------------------------------------------------
 * draw()
 * ------------------------------------------------------------
 * Перерисовывается ТОЛЬКО при изменении состояния
 */
void StatusBar::draw() {

    // если время стало валидным впервые — форсируем перерисовку даты
    if (!_wasTimeValid && _time.isValid()) {
        _lastDay   = -1;
        _lastMonth = -1;
        _lastYear  = -1;
    }

    if (!isDirty()) {
        _wasTimeValid = _time.isValid();
        return;
    }

    drawBackground();
    drawIcons();
    drawDate();

    // фиксируем текущее состояние
    _lastWifiOk   = _wifiOk;
    _lastTimeOk   = _time.isValid();
    _lastIsNight  = _theme.isNight();
    _wasTimeValid = _time.isValid();

    if (_time.isValid()) {
        _lastDay   = _time.day();
        _lastMonth = _time.month();
        _lastYear  = _time.year();
    }
}

/*
 * Фон статусбара
 */
void StatusBar::drawBackground() {
    const Theme& th = _theme.current();

    _tft.fillRect(
        0,
        0,
        _tft.width(),
        STATUS_BAR_H,
        th.bg
    );

    _tft.drawFastHLine(
        0,
        STATUS_BAR_H - 1,
        _tft.width(),
        th.accent
    );
}

/*
 * Иконки (БЕЗ фонового цвета!)
 */
void StatusBar::drawIcons() {
    const Theme& th = _theme.current();
    _tft.setFont(&FreeSans9pt7b);

    _tft.setTextColor(_wifiOk ? th.primary : th.secondary);
    _tft.setCursor(WIFI_X, ICON_Y);
    _tft.print("W");

    _tft.setTextColor(_time.isValid() ? th.primary : th.secondary);
    _tft.setCursor(NTP_X, ICON_Y);
    _tft.print("N");
}

/*
 * Дата (БЕЗ фонового цвета!)
 */
void StatusBar::drawDate() {
    if (!_time.isValid()) return;

    const Theme& th = _theme.current();
    _tft.setFont(&FreeSans9pt7b);
    _tft.setTextColor(th.secondary);

    static const char* DOW[] = {
        "ВС","ПН","ВТ","СР","ЧТ","ПТ","СБ"
    };

    char buf[24];
    snprintf(
        buf,
        sizeof(buf),
        "%s %02d.%02d.%04d",
        DOW[_time.weekday()],
        _time.day(),
        _time.month(),
        _time.year()
    );

    _tft.setCursor(DATE_X, ICON_Y);
    _tft.print(buf);
}