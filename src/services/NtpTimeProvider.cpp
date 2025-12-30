#include "services/NtpTimeProvider.h"
#include <Arduino.h>
#include <WiFi.h>

NtpTimeProvider::NtpTimeProvider() {
}

bool NtpTimeProvider::systemTimeLooksValid(const tm& t) const {
    // Нужна простая эвристика "это похоже на реальное время":
    //  - год >= 2020 (как и в твоём RTC проверяем разумность)
    const int y = t.tm_year + 1900;
    return (y >= 2020);
}

void NtpTimeProvider::update() {

    if (_ready) return;

    if (_requireWifi) {
        if (WiFi.status() != WL_CONNECTED) {
            return;
        }
    }

    tm t{};
    if (!getLocalTime(&t)) {
        return;
    }

    if (!systemTimeLooksValid(t)) {
        return;
    }

    _tm = t;
    _ready = true;
}

bool NtpTimeProvider::hasTime() const {
    return _ready;
}

TimeResult NtpTimeProvider::takeTime() {
    TimeResult r;
    r.valid = _ready;
    if (_ready) {
        r.time = _tm;
        _ready = false; // одноразовая выдача
    }
    return r;
}