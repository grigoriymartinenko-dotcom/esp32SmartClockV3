#include "ui/UiAnimator.h"

// ============================================================================
// UiAnimation
// ============================================================================
UiAnimation::UiAnimation()
    : _startMs(0)
    , _durationMs(0)
    , _active(false)
{
}

void UiAnimation::start(uint16_t durationMs) {
    _startMs   = millis();
    _durationMs = durationMs;
    _active    = true;
}

void UiAnimation::stop() {
    _active = false;
}

bool UiAnimation::active() const {
    return _active;
}

bool UiAnimation::finished() const {
    if (!_active) return true;
    return (millis() - _startMs) >= _durationMs;
}

float UiAnimation::progress() const {
    if (!_active) return 1.0f;

    uint32_t elapsed = millis() - _startMs;
    if (elapsed >= _durationMs) return 1.0f;

    return (float)elapsed / (float)_durationMs;
}

uint8_t UiAnimation::alpha() const {
    float p = progress();
    if (p <= 0.0f) return 0;
    if (p >= 1.0f) return 255;
    return (uint8_t)(p * 255.0f);
}

// ============================================================================
// UiAnimator
// ============================================================================
UiAnimator::UiAnimator()
    : fadeIn()
    , fadeOut()
{
}