#pragma once
#include "theme/Theme.h"

/*
 * Themes
 * ------
 * Конкретные цветовые схемы.
 * ПОРЯДОК ПОЛЕЙ ДОЛЖЕН СОВПАДАТЬ с Theme.h
 */

// =======================
// DAY THEME
// =======================
constexpr Theme THEME_DAY = {
    .bg = 0x0000,             // black

    .textPrimary   = 0x07E0,  // green
    .textSecondary = 0xFFFF, // white
    .muted         = 0x8410, // gray

    .accent = 0x001F,        // blue
    .error  = 0xF800         // red
};

// =======================
// NIGHT THEME
// =======================
constexpr Theme THEME_NIGHT = {
    .bg = 0x0000,             // black

    .textPrimary   = 0x03E0,  // dark green
    .textSecondary = 0x7BEF, // dim white
    .muted         = 0x4208, // dark gray

    .accent = 0x0010,        // dim blue
    .error  = 0x7800         // dark red
};