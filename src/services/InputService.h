#pragma once
#include <Arduino.h>

/*
 * InputService
 * ------------
 * Универсальный сервис кнопок.
 *
 * Знает:
 *  - GPIO
 *  - debounce
 *  - short / long press
 *
 * НЕ знает:
 *  - про экраны
 *  - про UI
 *  - про NAV / EDIT
 */

class InputService {
public:
    enum class Button : uint8_t {
        LEFT = 0,
        RIGHT,
        OK,
        BACK,
        COUNT
    };

    enum class Event : uint8_t {
        NONE,
        SHORT,
        LONG
    };

    struct Message {
        Button button;
        Event  event;
    };

public:
    InputService(
        uint8_t pinLeft,
        uint8_t pinRight,
        uint8_t pinOk,
        uint8_t pinBack
    );

    void begin();
    void update();

    // true если есть новое событие
    bool poll(Message& out);

private:
    struct BtnState {
        uint8_t pin;
        bool    lastLevel;
        bool    pressed;
        uint32_t pressMs;
        bool    longSent;
    };

private:
    BtnState _btn[(int)Button::COUNT];
};