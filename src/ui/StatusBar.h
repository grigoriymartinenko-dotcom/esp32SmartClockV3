#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"
#include "services/TimeService.h"

/*
 * StatusBar
 * ---------
 * Верхняя статусная панель:
 *  - WiFi  (OFFLINE / CONNECTING / ONLINE / ERROR)
 *  - Date  (DD.MM.YYYY)
 *  - Time source: RTC / NTP
 *
 * ПРАВИЛА:
 *  - НЕТ таймеров
 *  - НЕТ millis()
 *  - Рисует ТОЛЬКО по dirty-флагу
 *  - Вся логика "когда обновлять" — СНАРУЖИ
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

    // ===== реактивное обновление =====
    void update();
    void markDirty();

    // ===== события статусов =====
    void setWiFiStatus(Status s);
    void setNtpStatus(Status s);   // остаётся для CONNECTING / ERROR

private:
    void draw();

    // утилиты
    const char* statusText(Status s) const;
    uint16_t statusColor(Status s, const Theme& th) const;
    const char* timeSourceText(TimeService::Source s) const;

private:
    Adafruit_ST7735& _tft;
    ThemeService&    _theme;
    TimeService&     _time;

    Status _wifi = OFFLINE;
    Status _ntp  = OFFLINE;

    bool _dirty = true;
};