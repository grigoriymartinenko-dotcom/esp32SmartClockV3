#pragma once
#include <Arduino.h>

/*
 * DhtService
 * ----------
 * Сервис для работы с DHT11.
 *
 * Задачи:
 *  - читать датчик НЕ ЧАЩЕ 1 раза в ~3 сек
 *  - фильтровать "скачки"
 *  - сообщать, есть ли валидные данные
 *
 * ВАЖНО:
 *  - сервис НЕ рисует
 *  - сервис НЕ знает про экран
 */

class DhtService {
public:
    // инициализация датчика
    void begin(uint8_t pin);

    // периодическое обновление (вызывается часто — внутри стоит защита)
    void update();

    // true, если хотя бы одно валидное измерение уже есть
    bool valid() const;

    float temperature() const;
    float humidity() const;

private:
    uint32_t _lastRead = 0;

    // "сырые" значения прямо из датчика
    float _tempRaw = NAN;
    float _humRaw  = NAN;

    // сглаженные значения (то, что отдаём наружу)
    float _tempAvg = NAN;
    float _humAvg  = NAN;

    // флаг: данные реально получены
    bool  _hasData = false;

    void readSensor();
};