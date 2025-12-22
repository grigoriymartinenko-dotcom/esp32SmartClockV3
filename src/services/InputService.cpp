#include "services/InputService.h"

static constexpr uint32_t DEBOUNCE_MS = 30;
static constexpr uint32_t LONG_MS     = 600;

InputService::InputService(
    uint8_t pinLeft,
    uint8_t pinRight,
    uint8_t pinOk,
    uint8_t pinBack
) {
    _btn[(int)Button::LEFT]  = { pinLeft,  HIGH, false, 0, false };
    _btn[(int)Button::RIGHT] = { pinRight, HIGH, false, 0, false };
    _btn[(int)Button::OK]    = { pinOk,    HIGH, false, 0, false };
    _btn[(int)Button::BACK]  = { pinBack,  HIGH, false, 0, false };
}

void InputService::begin() {
    for (int i = 0; i < (int)Button::COUNT; i++) {
        pinMode(_btn[i].pin, INPUT_PULLUP);
        _btn[i].lastLevel = digitalRead(_btn[i].pin);
    }
}

void InputService::update() {
    // ничего — логика в poll()
}

bool InputService::poll(Message& out) {
    uint32_t now = millis();

    for (int i = 0; i < (int)Button::COUNT; i++) {
        BtnState& b = _btn[i];
        bool level = digitalRead(b.pin);

        // debounce
        if (level != b.lastLevel) {
            b.lastLevel = level;
            delayMicroseconds(200); // очень короткий антидребезг
            continue;
        }

        // нажали
        if (!b.pressed && level == LOW) {
            b.pressed  = true;
            b.pressMs  = now;
            b.longSent = false;
        }

        // держим
        if (b.pressed && !b.longSent && level == LOW) {
            if (now - b.pressMs >= LONG_MS) {
                b.longSent = true;
                out = { (Button)i, Event::LONG };
                return true;
            }
        }

        // отпустили
        if (b.pressed && level == HIGH) {
            b.pressed = false;

            if (!b.longSent && (now - b.pressMs) > DEBOUNCE_MS) {
                out = { (Button)i, Event::SHORT };
                return true;
            }
        }
    }

    return false;
}