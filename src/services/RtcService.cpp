#include "services/RtcService.h"
#include <DS1302.h>

// ================= ctor =================

RtcService::RtcService(int clkPin, int datPin, int rstPin)
    : _clk(clkPin)
    , _dat(datPin)
    , _rst(rstPin)
{
}

// ================= lifecycle =================

void RtcService::begin() {
    static DS1302 rtc(_rst, _dat, _clk);
    rtc.halt(false);
    rtc.writeProtect(false);
}

// ================= write =================

void RtcService::write(const tm& in) {
    static DS1302 rtc(_rst, _dat, _clk);

    Time t(
        in.tm_year + 1900,
        in.tm_mon + 1,
        in.tm_mday,
        in.tm_hour,
        in.tm_min,
        in.tm_sec,
        static_cast<Time::Day>(1) // Day of week, DS1302 не критично
    );

    rtc.time(t);
}

// ================= read =================
bool RtcService::read(tm& out) {
    static DS1302 rtc(_rst, _dat, _clk);

    Time t = rtc.time();

    // DS1302 не имеет isValid → проверяем разумность
    if (t.yr < 2020 || t.mon < 1 || t.mon > 12)
        return false;

    out = {};
    out.tm_year = t.yr - 1900;
    out.tm_mon  = t.mon - 1;
    out.tm_mday = t.date;
    out.tm_hour = t.hr;
    out.tm_min  = t.min;
    out.tm_sec  = t.sec;

    return true;
}