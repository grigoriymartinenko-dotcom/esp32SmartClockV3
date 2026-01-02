#pragma once
#include "theme/Theme.h"

/*
 * Themes
 * ------
 * Конкретные цветовые схемы.
 *
 * Концепция:
 *  - textPrimary  = основной текст
 *  - select       = активный пункт меню
 *  - accent       = линии / разделители
 *  - muted        = вторичное
 */

// =======================
// DAY THEME
// =======================
constexpr Theme THEME_DAY = {
    .bg = 0x0861,             // тёмно-синий фон

    .textPrimary   = 0x7DDF,  // мягкий циан
    .textSecondary = 0x4C7F,  // зелёный
//    .textSecondary = 0x57EA,  // зелёный
    .muted         = 0x52B4,  // серо-синий

    .select        = 0xcd1,  // ЯРТО-красный (курсор)

    .warn          = 0xF145,  // красный (edit / warning)
    .accent        = 0xd8E3,  // линии / разделители
    .error         = 0xF145
};

// =======================
// NIGHT THEME
// =======================
constexpr Theme THEME_NIGHT = {
    .bg = 0x0000,             // почти чёрный

    .textPrimary   = 0x4C7F,  // приглушённый циан
    .textSecondary = 0x3D66,  // тёмно-зелёный
    .muted         = 0x4208,  // тёмно-серый

    .select        = 0xcd1,  // ЯРТО-красный (курсор)

    .warn          = 0xF145,  // warning
    .accent        = 0x0841,  // тёмные линии
    .error         = 0x7800
};