#include "services/NightTransitionService.h"

// ============================================================================
// ctor
// ============================================================================

NightTransitionService::NightTransitionService()
    : _targetNight(false)
    , _factor(0.0f)
    , _lastMs(0)
{
}

// ============================================================================
// control
// ============================================================================

void NightTransitionService::setTarget(bool night) {
    _targetNight = night;
}

void NightTransitionService::update() {
    uint32_t now = millis();

    // Первый вызов — просто инициализируем таймер,
    // чтобы не было скачка.
    if (_lastMs == 0) {
        _lastMs = now;
        return;
    }

    uint32_t dt = now - _lastMs;
    _lastMs = now;

    // На сколько меняем коэффициент за этот кадр
    float delta = dt * SPEED;

    if (_targetNight) {
        // Движемся к ночи
        _factor += delta;
        if (_factor > 1.0f) {
            _factor = 1.0f;
        }
    } else {
        // Движемся к дню
        _factor -= delta;
        if (_factor < 0.0f) {
            _factor = 0.0f;
        }
    }
}

// ============================================================================
// state
// ============================================================================

bool NightTransitionService::transitioning() const {
    // Переход активен, если:
    //  - идём в ночь, но ещё не дошли до 1.0
    //  - идём в день, но ещё не дошли до 0.0
    return (_targetNight && _factor < 1.0f)
        || (!_targetNight && _factor > 0.0f);
}

float NightTransitionService::nightFactor() const {
    return _factor;
}

float NightTransitionService::value() const {
    // Универсальный коэффициент для blend'а
    return _factor;
}