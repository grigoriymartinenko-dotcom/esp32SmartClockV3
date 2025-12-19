#pragma once
#include <Arduino.h>
#include <DHT.h>

/*
 * DhtService
 * ----------
 * Периодически читает DHT и хранит значения.
 */
class DhtService {
public:
    DhtService(uint8_t pin, uint8_t type);

    void begin();
    void update();

    bool   isValid() const;
    float  temperature() const;  // °C
    float  humidity() const;     // %

private:
    DHT _dht;

    float _temp = NAN;
    float _hum  = NAN;

    uint32_t _lastReadMs = 0;
    static constexpr uint32_t READ_INTERVAL_MS = 3000;
};