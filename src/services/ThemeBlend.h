#pragma once
#include <stdint.h>

/*
 * ThemeBlend.h
 * -----------
 * ThemeBlend — "снимок" готовых цветов UI после смешивания Day/Night.
 *
 * ВАЖНО:
 *  - Это НЕ сервис, НЕ singleton, НЕ "тема".
 *  - Это просто набор 16-битных RGB565 цветов.
 *  - UI (StatusBar / Screens) должен работать ТОЛЬКО с ThemeBlend,
 *    не зная ничего о Day/Night / AUTO/ON/OFF.
 *
 * Типичный пайплайн:
 *   ThemeBlend th = themeService.interpolate(t);
 *   th = colorTemp.apply(th);
 *   draw(th);
 */

struct ThemeBlend {
    uint16_t bg;       // background
    uint16_t fg;       // primary text / main foreground
    uint16_t accent;   // accent color (highlights)
    uint16_t muted;    // secondary text / subtle elements
    uint16_t warn;     // warnings / attention
    uint16_t success;  // success / ok states
};