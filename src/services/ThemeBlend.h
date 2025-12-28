#pragma once
#include <stdint.h>

/*
 * ThemeBlend.h
 * ------------
 * ThemeBlend — "снимок" готовых цветов UI после смешивания Day/Night.
 *
 * ВАЖНО:
 *  - Это НЕ сервис
 *  - Это НЕ тема
 *  - Это просто набор RGB565 цветов для UI
 *
 * UI (экраны / StatusBar):
 *  - НЕ знает Day/Night
 *  - НЕ знает коэффициентов
 *  - просто рисует теми цветами, которые ему дали
 *
 * Типичный пайплайн:
 *   ThemeBlend th = themeService.interpolate(k);
 *   draw(th);
 */

struct ThemeBlend {
    uint16_t bg;       // background
    uint16_t fg;       // основной текст / иконки (нейтральный)
    uint16_t accent;   // акценты
    uint16_t muted;    // вторичный текст
    uint16_t warn;     // предупреждения / ошибки
    uint16_t success;  // OK / success

    // ------------------------------------------------------------
    // Color temperature
    // ------------------------------------------------------------
    uint16_t fgWarm;   // тёплый foreground (ночь)
    uint16_t fgCool;   // холодный foreground (день)
};