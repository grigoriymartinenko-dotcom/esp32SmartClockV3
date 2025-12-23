#pragma once
#include <stdint.h>

/*
 * Theme
 * -----
 * Цветовая схема UI.
 *
 * ПОРЯДОК ПОЛЕЙ ВАЖЕН!
 */
struct Theme {
    uint16_t bg;

    uint16_t textPrimary;
    uint16_t textSecondary;
    uint16_t muted;

    uint16_t select;   // <-- ВЫБОР МЕНЮ (НОВОЕ)
    uint16_t warn;
    uint16_t accent;
    uint16_t error;
};