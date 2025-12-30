#include "services/RtcTimeProvider.h"

RtcTimeProvider::RtcTimeProvider(RtcService& rtc)
    : _rtc(rtc)
{
}

void RtcTimeProvider::update() {
    if (_done) return;
    _done = true;

    tm t{};
    if (_rtc.read(t)) {
        _tm = t;
        _ready = true;
    }
}

bool RtcTimeProvider::hasTime() const {
    return _ready;
}

TimeResult RtcTimeProvider::takeTime() {
    TimeResult r;
    r.valid = _ready;
    if (_ready) {
        r.time = _tm;
        _ready = false; // одноразовая выдача
    }
    return r;
}