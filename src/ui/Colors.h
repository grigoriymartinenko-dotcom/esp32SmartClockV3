#pragma once
#include "theme/Themes.h"

constexpr uint16_t C_UI_BG_DAY   = THEME_DAY.bg;
constexpr uint16_t C_UI_BG_NIGHT = THEME_NIGHT.bg;
constexpr uint16_t C_UI_ACCENT   = THEME_DAY.accent;

constexpr uint32_t C_STATUS_WIFI_OK  = 0x59ff0000;
constexpr uint32_t C_STATUS_WIFI_ERR = 0xff001a00;
constexpr uint16_t C_STATUS_NTP_OK   = 0x07E0;
constexpr uint16_t C_STATUS_NTP_WAIT = 0xFFE0;
