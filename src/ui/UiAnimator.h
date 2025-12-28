#pragma once

#include <Arduino.h>

/*
 * UiAnimator
 * ----------
 * Универсальный анимационный модуль UI.
 *
 * ЗАДАЧА:
 *  - дать UI простые параметры (alpha, progress)
 *  - убрать тайминги и состояния из экранов
 *
 * ВАЖНО:
 *  - не знает про TFT
 *  - не знает про экраны
 *  - не рисует
 */

// ============================================================================
// Base animation
// ============================================================================
class UiAnimation {
public:
    UiAnimation();

    void start(uint16_t durationMs);
    void stop();

    bool active() const;
    bool finished() const;

    float progress() const;   // 0.0 .. 1.0
    uint8_t alpha() const;    // 0 .. 255

private:
    uint32_t _startMs;
    uint16_t _durationMs;
    bool     _active;
};

// ============================================================================
// Animator container
// ============================================================================
class UiAnimator {
public:
    UiAnimator();

    UiAnimation fadeIn;
    UiAnimation fadeOut;
};