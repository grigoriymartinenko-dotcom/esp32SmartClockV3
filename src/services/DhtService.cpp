#include "services/DhtService.h"

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

    _hum  = h;
    _temp = t;
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