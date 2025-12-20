#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"
#include "services/TimeService.h"

/*
 * StatusBar
 * ---------
 * Верхняя статусная панель:
 *  - Wi-Fi статус
 *  - дата
 *  - NTP статус
 *
 * ПРАВИЛА:
 *  - НЕТ таймеров
 *  - НЕТ millis()
 *  - Рисует ТОЛЬКО по dirty-флагу
 *  - Вся логика "когда" — СНАРУЖИ
 */
class StatusBar {
public:
    static constexpr int HEIGHT = 24;

    enum Status {
        OFFLINE,
        CONNECTING,
        ONLINE,
        ERROR
    };

    StatusBar(
        Adafruit_ST7735& tft,
        ThemeService& theme,
        TimeService& time
    );

    // 🔹 реактивное обновление
    void update();

    // 🔹 пометить на перерисовку (всё)
    void markDirty();

    // 🔹 события статусов
    void setWiFiStatus(Status s);
    void setNtpStatus(Status s);

private:
    void draw();        // рисует ВСЮ панель
    char statusChar(Status s) const;
    uint16_t statusColor(Status s, const Theme& th) const;

    Adafruit_ST7735& _tft;
    ThemeService&    _theme;
    TimeService&     _time;

    Status _wifi = OFFLINE;
    Status _ntp  = OFFLINE;

    bool _dirty = true;
};