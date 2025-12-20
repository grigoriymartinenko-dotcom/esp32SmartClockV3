#include "services/DhtService.h"
#include <math.h>

DhtService::DhtService(uint8_t pin, uint8_t type)
: _dht(pin, type)
{}

void DhtService::begin() {
    _dht.begin();
}

void DhtService::update() {
    uint32_t now = millis();
    if (now - _lastReadMs < READ_INTERVAL_MS)
        return;

    _lastReadMs = now;

    float h = _dht.readHumidity();
    float t = _dht.readTemperature();

    if (isnan(h) || isnan(t))
        return;

    bool changed = false;

    // сравниваем с предыдущими значениями
    if (isnan(_temp) || fabs(_temp - t) >= 0.1f) {
        _temp = t;
        changed = true;
    }

    if (isnan(_hum) || fabs(_hum - h) >= 0.5f) {
        _hum = h;
        changed = true;
    }

    if (changed) {
        _version.bump();
    }
}

bool DhtService::isValid() const {
    return !isnan(_temp) && !isnan(_hum);
}

float DhtService::temperature() const {
    return _temp;
}

float DhtService::humidity() const {
    return _hum;
}

const ServiceVersion& DhtService::version() const {
    return _version;
}