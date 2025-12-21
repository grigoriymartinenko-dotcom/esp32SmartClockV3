#include "input/Buttons.h"

Buttons::Buttons(
    uint8_t pinLeft,
    uint8_t pinRight,
    uint8_t pinOk,
    uint8_t pinBack,
    uint32_t debounceMs,
    uint32_t longPressMs
)
    : _debounceMs(debounceMs)
    , _longPressMs(longPressMs)
{
    _left.pin  = pinLeft;
    _right.pin = pinRight;
    _ok.pin    = pinOk;
    _back.pin  = pinBack;
}

void Buttons::Btn::begin(uint8_t p) {
    pin = p;
    pinMode(pin, INPUT_PULLUP);

    bool r = digitalRead(pin);
    stable = r;
    lastRaw = r;
    lastChangeMs = 0;

    isDown = (stable == LOW);
    downSinceMs = 0;
    longFired = false;
}

void Buttons::Btn::updateRaw(bool raw, uint32_t nowMs, uint32_t debounceMs) {
    // фиксируем изменения сырого сигнала
    if (raw != lastRaw) {
        lastRaw = raw;
        lastChangeMs = nowMs;
    }

    // если сырой сигнал стабилен достаточное время — принимаем новое stable
    if ((nowMs - lastChangeMs) >= debounceMs && stable != lastRaw) {
        stable = lastRaw;

        // переходы stable:
        if (stable == LOW) {
            // НАЖАТИЕ (down)
            isDown = true;
            downSinceMs = nowMs;
            longFired = false;
        } else {
            // ОТПУСКАНИЕ (up)
            isDown = false;
        }
    }
}

bool Buttons::readEventFor(Btn& b, ButtonId id, uint32_t nowMs, ButtonEvent& out) {

    // Long press: один раз за удержание
    if (b.isDown && !b.longFired) {
        if ((nowMs - b.downSinceMs) >= _longPressMs) {
            b.longFired = true;
            out = { id, ButtonEventType::LONG_PRESS };
            return true;
        }
    }

    // Short press: событие на отпускании, если long не было
    // (то есть "клик" считается завершённым только на release)
    // Важно: мы не используем отдельный флаг "released" — его можно поймать так:
    // stable==HIGH и при этом lastRaw==HIGH и isDown==false после перехода
    // Но нам нужно сработать ОДИН раз. Для этого используем трюк:
    // - если stable==HIGH, isDown==false, и longFired==false, и downSinceMs != 0,
    //   значит был клик и отпустили.
    if (!b.isDown && !b.longFired && b.downSinceMs != 0) {
        // генерим short и сбрасываем downSinceMs чтобы не повторялось
        b.downSinceMs = 0;
        out = { id, ButtonEventType::SHORT_PRESS };
        return true;
    }

    return false;
}

void Buttons::begin() {
    _left.begin(_left.pin);
    _right.begin(_right.pin);
    _ok.begin(_ok.pin);
    _back.begin(_back.pin);
}

bool Buttons::poll(ButtonEvent& out) {
    const uint32_t now = millis();

    // обновляем stable состояния
    _left.updateRaw(digitalRead(_left.pin), now, _debounceMs);
    _right.updateRaw(digitalRead(_right.pin), now, _debounceMs);
    _ok.updateRaw(digitalRead(_ok.pin), now, _debounceMs);
    _back.updateRaw(digitalRead(_back.pin), now, _debounceMs);

    // порядок приоритетов событий (чтобы не было "одновременно")
    // OK/BACK обычно важнее для UX, поэтому они раньше
    if (readEventFor(_ok, ButtonId::OK, now, out)) return true;
    if (readEventFor(_back, ButtonId::BACK, now, out)) return true;
    if (readEventFor(_left, ButtonId::LEFT, now, out)) return true;
    if (readEventFor(_right, ButtonId::RIGHT, now, out)) return true;

    return false;
}