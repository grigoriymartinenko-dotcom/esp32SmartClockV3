#include "input/Buttons.h"

Buttons::Buttons(
    uint8_t pinLeft,
    uint8_t pinRight,
    uint8_t pinOk,
    uint8_t pinBack,
    uint32_t debounceMs
)
    : _debounceMs(debounceMs)
{
    _left.pin  = pinLeft;
    _right.pin = pinRight;
    _ok.pin    = pinOk;
    _back.pin  = pinBack;
}

void Buttons::Btn::begin(uint8_t p) {
    pin = p;
    pinMode(pin, INPUT_PULLUP);
    last = digitalRead(pin);
    lastMs = 0;
}

bool Buttons::Btn::pressed(uint32_t nowMs, uint32_t debounceMs) {
    bool v = digitalRead(pin);
    bool trig = false;

    if (last == HIGH && v == LOW && (nowMs - lastMs) > debounceMs) {
        trig = true;
        lastMs = nowMs;
    }

    last = v;
    return trig;
}

void Buttons::begin() {
    _left.begin(_left.pin);
    _right.begin(_right.pin);
    _ok.begin(_ok.pin);
    _back.begin(_back.pin);
}

ButtonsState Buttons::poll() {
    const uint32_t now = millis();

    ButtonsState st;
    st.left  = _left.pressed(now, _debounceMs);
    st.right = _right.pressed(now, _debounceMs);
    st.ok    = _ok.pressed(now, _debounceMs);
    st.back  = _back.pressed(now, _debounceMs);

    return st;
}