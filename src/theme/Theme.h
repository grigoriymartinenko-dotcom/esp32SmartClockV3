#pragma once
#include <stdint.h>

/*
 * Theme
 * -----
 * Все цвета UI.
 * ПОРЯДОК ПОЛЕЙ КРИТИЧЕН для C++ designated initializers!
 */
struct Theme {
    uint16_t bg;

    uint16_t textPrimary;    // 🟢 OK
    uint16_t textSecondary;
    uint16_t muted;

    uint16_t accent;         // 🔵 процесс
    uint16_t error;          // 🔴 ошибка
};