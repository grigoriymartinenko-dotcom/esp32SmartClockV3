#include "services/NightTransitionService.h"

#include <math.h>

// ============================================================================
// ctor
// ============================================================================

NightTransitionService::NightTransitionService()
    : _targetNight(false)
    , _t(0.0f)
    , _lastMs(0)
    , _dirty(true)   // чтобы первый кадр гарантированно отрисовался
    , _lastQ(255)    // заведомо "не равно" quantize8(0.0)
{
}

// ============================================================================
// helpers
// ============================================================================

float NightTransitionService::smoothstep(float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

uint8_t NightTransitionService::quantize8(float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    int v = (int)lroundf(t * 255.0f);
    if (v < 0) v = 0;
    if (v > 255) v = 255;
    return (uint8_t)v;
}

// ============================================================================
// control
// ============================================================================

void NightTransitionService::setTarget(bool night) {
    _targetNight = night;
    // Само по себе изменение цели не означает, что value() изменилось,
    // поэтому dirty тут НЕ ставим. dirty ставится в update() при реальном
    // изменении t/value.
}

void NightTransitionService::update() {
    const uint32_t now = millis();

    // Первый вызов: просто инициализируем таймер.
    if (_lastMs == 0) {
        _lastMs = now;
        _dirty = true;          // первый кадр/итерация
        _lastQ = quantize8(value());
        return;
    }

    uint32_t dtMs = now - _lastMs;
    _lastMs = now;

    // Страховка: если dt вдруг огромный (пауза/зависание),
    // ограничим шаг, чтобы не "телепортировать" переход.
    if (dtMs > 250) dtMs = 250;

    const float prevValue = value();

    // Линейно двигаем t к цели.
    if (_targetNight) {
        _t += (float)dtMs * SPEED;
        if (_t >= 1.0f - SNAP_EPS) _t = 1.0f;
    } else {
        _t -= (float)dtMs * SPEED;
        if (_t <= 0.0f + SNAP_EPS) _t = 0.0f;
    }

    // Dirty-логика: считаем "заметным" изменение по квантованию 0..255
    // уже после easing. Это ближе к тому, как глаз видит изменение.
    const float curValue = value();
    const uint8_t q = quantize8(curValue);

    // dirty если:
    //  - поменялся квант
    //  - или это самый первый апдейт/кадр
    _dirty = (q != _lastQ) || (prevValue != curValue);
    _lastQ = q;
}

// ============================================================================
// state
// ============================================================================

bool NightTransitionService::transitioning() const {
    return (_targetNight && _t < 1.0f)
        || (!_targetNight && _t > 0.0f);
}

bool NightTransitionService::dirty() const {
    return _dirty;
}

void NightTransitionService::clearDirty() {
    _dirty = false;
}

bool NightTransitionService::targetNight() const {
    return _targetNight;
}

float NightTransitionService::rawFactor() const {
    return _t;
}

float NightTransitionService::nightFactor() const {
    // Для семантики и обратной совместимости.
    return value();
}

float NightTransitionService::value() const {
    // Визуальный easing
    return smoothstep(_t);
}