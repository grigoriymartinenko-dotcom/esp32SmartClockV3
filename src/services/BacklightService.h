#pragma once
#include <stdint.h>

/*
 * BacklightService
 * ----------------
 * Управляет ФИЗИЧЕСКОЙ подсветкой TFT через PWM (ESP32 LEDC).
 *
 * ВАЖНОЕ разделение ответственности:
 *
 *  - BrightnessService  → меняет яркость ЦВЕТОВ UI (ThemeBlend pipeline)
 *  - BacklightService   → меняет яркость ПОДСВЕТКИ МАТРИЦЫ (реальный свет)
 *
 * Этот сервис:
 *  - НЕ знает ничего про ThemeBlend / ThemeService / экраны
 *  - НЕ рисует UI
 *  - просто держит значение 0.0 .. 1.0 и пишет PWM в TFT_BL
 *
 * Требование по железу:
 *  - TFT_BL должен быть подключён к GPIO (часто через транзистор/MOSFET),
 *    иначе (если он сидит на 3.3V) яркость программно не изменится.
 */

class BacklightService {
public:
    void begin();

    // value: 0.0f .. 1.0f
    void set(float value);
    float get() const;

private:
    void apply();

private:
    float _value = 1.0f;
};