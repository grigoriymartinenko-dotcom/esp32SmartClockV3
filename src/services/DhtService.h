#pragma once
#include <Arduino.h>
#include <DHT.h>
#include "core/ServiceVersion.h"

/*
 * DhtService
 * ----------
 * Периодически читает DHT и хранит значения.
 * Реактивный сервис (через versioning)
 */
class DhtService {
public:
    DhtService(uint8_t pin, uint8_t type);

    void begin();
    void update();

    bool   isValid() const;
    float  temperature() const;  // °C
    float  humidity() const;     // %

    // 🔥 VERSION
    const ServiceVersion& version() const;

private:
    DHT _dht;

    float _temp = NAN;
    float _hum  = NAN;

    uint32_t _lastReadMs = 0;
    static constexpr uint32_t READ_INTERVAL_MS = 3000;

    ServiceVersion _version;
};