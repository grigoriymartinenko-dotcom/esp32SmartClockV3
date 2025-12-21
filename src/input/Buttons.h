#pragma once
#include <Arduino.h>

/*
 * Buttons
 * -------
 * Обёртка над 4 аппаратными кнопками с debounce.
 *
 * Правила:
 *  - НЕТ логики экранов
 *  - НЕТ логики UI
 *  - Только состояние кнопок
 */

struct ButtonsState {
    bool left  = false;
    bool right = false;
    bool ok    = false;
    bool back  = false;
};

class Buttons {
public:
    Buttons(
        uint8_t pinLeft,
        uint8_t pinRight,
        uint8_t pinOk,
        uint8_t pinBack,
        uint32_t debounceMs = 200
    );

    void begin();
    ButtonsState poll();   // вызывать 1 раз за loop

private:
    struct Btn {
        uint8_t pin = 0;
        bool last = HIGH;
        uint32_t lastMs = 0;

        void begin(uint8_t p);
        bool pressed(uint32_t nowMs, uint32_t debounceMs);
    };

private:
    Btn _left;
    Btn _right;
    Btn _ok;
    Btn _back;

    uint32_t _debounceMs;
};