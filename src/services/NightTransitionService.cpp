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
    if (_lastMs == 0) {
        _lastMs = now;
        return;
    }

    uint32_t dt = now - _lastMs;
    _lastMs = now;

    float delta = dt * SPEED;

    if (_targetNight) {
        _factor += delta;
        if (_factor > 1.0f) _factor = 1.0f;
    } else {
        _factor -= delta;
        if (_factor < 0.0f) _factor = 0.0f;
    }
}

// ============================================================================
// state
// ============================================================================
bool NightTransitionService::transitioning() const {
    return (_targetNight && _factor < 1.0f)
        || (!_targetNight && _factor > 0.0f);
}

float NightTransitionService::nightFactor() const {
    return _factor;
}