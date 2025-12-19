#include "services/DhtService.h"
#include <DHT.h>

/*
 * DHT11:
 *  - работает медленно
 *  - часто отдаёт NaN
 *  - нельзя читать чаще ~2 сек
 */

#define DHT_TYPE DHT11

// Статический объект — без new/delete (важно для стабильности)
static DHT dht(25, DHT_TYPE);

void DhtService::begin(uint8_t /*pin*/) {
    dht.begin();
    _lastRead = 0;
}

void DhtService::update() {
    // защита от слишком частых чтений
    if (millis() - _lastRead < 3000) return;
    _lastRead = millis();

    readSensor();

    // если датчик вернул мусор — просто выходим
    if (isnan(_tempRaw) || isnan(_humRaw)) return;

    // первое валидное измерение
    if (!_hasData) {
        _tempAvg = _tempRaw;
        _humAvg  = _humRaw;
        _hasData = true;
        return;
    }

    /*
     * Сглаживание:
     *  70% старое значение
     *  30% новое
     *
     * Это убирает резкие скачки DHT11
     */
    _tempAvg = _tempAvg * 0.7f + _tempRaw * 0.3f;
    _humAvg  = _humAvg  * 0.7f + _humRaw  * 0.3f;
}

void DhtService::readSensor() {
    _humRaw  = dht.readHumidity();
    _tempRaw = dht.readTemperature();
}

bool DhtService::valid() const {
    return _hasData;
}

float DhtService::temperature() const {
    return _tempAvg;
}

float DhtService::humidity() const {
    return _humAvg;
}