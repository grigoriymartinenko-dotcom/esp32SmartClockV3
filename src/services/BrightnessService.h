#pragma once

#include <stdint.h>

/*
 * BrightnessService
 * -----------------
 * ЕДИНЫЙ источник истины для яркости UI.
 *
 * Ответственность:
 *  - хранит значение яркости 0..100
 *  - загружает / сохраняет в Preferences
 *  - переводит 0..100 → аппаратный уровень (PWM)
 *
 * ВАЖНО:
 *  - НЕ знает про экраны
 *  - НЕ знает про Theme / Day / Night
 *  - НЕ дергает ledcWrite напрямую
 *
 * Связь с железом осуществляется через callback,
 * который задаётся в main.cpp или Display-init.
 */

class BrightnessService {
public:
    // callback: принимает "аппаратный" уровень (0..255 или иной)
    typedef void (*ApplyFn)(uint8_t hwValue);

public:
    BrightnessService();

    // загрузка значения из Preferences
    void begin();

    // установить яркость 0..100
    void set(uint8_t value);

    // получить текущую яркость 0..100
    uint8_t get() const;

    // применить текущее значение к железу
    void apply();

    // привязать функцию применения (PWM / backlight)
    void attach(ApplyFn fn);

private:
    uint8_t clamp(uint8_t v) const;

private:
    uint8_t _value = 100;   // 0..100, по умолчанию 100%
    ApplyFn _apply = nullptr;
};