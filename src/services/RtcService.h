#pragma once
#include <time.h>

/*
 * RtcService
 * ----------
 * Обёртка над DS1302.
 * Отвечает ТОЛЬКО за:
 *  - чтение времени из RTC
 *  - запись времени в RTC
 */

class RtcService {
public:
    // пины DS1302
    RtcService(int clkPin, int datPin, int rstPin);

    void begin();

    // вернуть true если время похоже на валидное
    bool read(tm& out);

    // записать время в RTC
    void write(const tm& in);

private:
    int _clk;
    int _dat;
    int _rst;
};