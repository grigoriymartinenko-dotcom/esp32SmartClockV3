#pragma once
#include <Arduino.h>

/*
 * Buttons (v2)
 * ------------
 * Слой ввода: 4 аппаратные кнопки + debounce + long-press.
 *
 * Правила:
 *  - UI/экраны НЕ знают про millis()
 *  - Buttons генерирует события: short/long
 *  - LongPress срабатывает ОДИН раз за удержание
 */

enum class ButtonId : uint8_t {
    LEFT = 0,
    RIGHT,
    OK,
    BACK
};

enum class ButtonEventType : uint8_t {
    SHORT_PRESS = 0,   // отпустили до long threshold
    LONG_PRESS         // удержали >= threshold (срабатывает один раз)
};

struct ButtonEvent {
    ButtonId id;
    ButtonEventType type;
};

class Buttons {
public:
    Buttons(
        uint8_t pinLeft,
        uint8_t pinRight,
        uint8_t pinOk,
        uint8_t pinBack,
        uint32_t debounceMs = 50,
        uint32_t longPressMs = 800
    );

    void begin();

    // poll() возвращает true, если есть событие (одно за вызов).
    // Внутри хранится очередь максимум 1 событие; вызывать в loop() часто.
    bool poll(ButtonEvent& out);

private:
    struct Btn {
        uint8_t pin = 0;

        // debounce
        bool stable = HIGH;
        bool lastRaw = HIGH;
        uint32_t lastChangeMs = 0;

        // press/hold
        bool isDown = false;
        uint32_t downSinceMs = 0;
        bool longFired = false;

        void begin(uint8_t p);
        void updateRaw(bool raw, uint32_t nowMs, uint32_t debounceMs);
    };

    bool readEventFor(Btn& b, ButtonId id, uint32_t nowMs, ButtonEvent& out);

private:
    Btn _left;
    Btn _right;
    Btn _ok;
    Btn _back;

    uint32_t _debounceMs;
    uint32_t _longPressMs;
};