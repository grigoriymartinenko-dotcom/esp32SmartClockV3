#pragma once
#include <stdint.h>

/*
 * UI COLOR PALETTE (RGB565)
 * ========================
 * Проект: ESP32 Clock / ST7735
 *
 * Правила:
 *  - НИКАКИХ ST7735_* / ST77XX_*
 *  - Только явные RGB565 значения
 *  - Используем эти имена везде
 */

/* ================= ОСНОВНЫЕ ================= */

constexpr uint16_t C_BLACK  = 0x0000;
constexpr uint16_t C_WHITE  = 0xFFFF;

constexpr uint16_t C_RED    = 0xF800;
constexpr uint16_t C_GREEN  = 0x07E0;
constexpr uint16_t C_BLUE   = 0x001F;

constexpr uint16_t C_CYAN   = 0x07FF;
constexpr uint16_t C_MAGENTA= 0xF81F;
constexpr uint16_t C_YELLOW = 0xFFE0;

/* ================= СЕРЫЕ ================= */

constexpr uint16_t C_GRAY_10 = 0x18E3;   // почти чёрный
constexpr uint16_t C_GRAY_20 = 0x3186;
constexpr uint16_t C_GRAY_30 = 0x4A49;
constexpr uint16_t C_GRAY_40 = 0x630C;
constexpr uint16_t C_GRAY_50 = 0x7BEF;   // нейтральный
constexpr uint16_t C_GRAY_60 = 0x94B2;
constexpr uint16_t C_GRAY_70 = 0xAD75;
constexpr uint16_t C_GRAY_80 = 0xC618;
constexpr uint16_t C_GRAY_90 = 0xDEF9;   // почти белый

/* ================= UI / АКЦЕНТЫ ================= */

constexpr uint16_t C_UI_BG_DAY    = 0xFFFF;   // белый
constexpr uint16_t C_UI_BG_NIGHT  = 0x0000;   // чёрный

constexpr uint16_t C_UI_TEXT_DAY  = 0x0000;
constexpr uint16_t C_UI_TEXT_NIGHT= 0xFFFF;

constexpr uint16_t C_UI_ACCENT    = 0x03EF;   // мягкий голубой
constexpr uint16_t C_UI_WARNING   = 0xFBE0;   // тёплый жёлтый
constexpr uint16_t C_UI_ERROR     = 0xF800;   // красный
constexpr uint16_t C_UI_OK        = 0x07E0;   // зелёный

/* ================= ВРЕМЯ / ЧАСЫ ================= */

constexpr uint16_t C_TIME_MAIN_DAY    = 0x0000;
constexpr uint16_t C_TIME_MAIN_NIGHT  = 0xFFFF;

constexpr uint16_t C_TIME_SECONDS_DAY = 0x001F;   // синий
constexpr uint16_t C_TIME_SECONDS_NIGHT = 0xFFE0; // жёлтый

constexpr uint16_t C_TIME_DATE_DAY    = 0x7BEF;   // серый
constexpr uint16_t C_TIME_DATE_NIGHT  = 0x07FF;   // циан

/* ================= ПОГОДА ================= */

constexpr uint16_t C_WEATHER_SUN   = 0xFFE0;
constexpr uint16_t C_WEATHER_CLOUD = 0xC618;
constexpr uint16_t C_WEATHER_RAIN  = 0x03EF;
constexpr uint16_t C_WEATHER_SNOW  = 0xFFFF;
constexpr uint16_t C_WEATHER_STORM = 0xF800;

/* ================= ГРАФИКИ ================= */

constexpr uint16_t C_GRAPH_LINE_1 = 0x07FF;   // циан
constexpr uint16_t C_GRAPH_LINE_2 = 0xF800;   // красный
constexpr uint16_t C_GRAPH_LINE_3 = 0x07E0;   // зелёный

constexpr uint16_t C_GRAPH_GRID   = 0x39E7;   // тёмно-серый
constexpr uint16_t C_GRAPH_TEXT   = 0xFFFF;

/* ================= КНОПКИ / ИКОНКИ ================= */

constexpr uint16_t C_BTN_ACTIVE   = 0x07E0;   // зелёный
constexpr uint16_t C_BTN_INACTIVE = 0x630C;   // серый
constexpr uint16_t C_BTN_HINT     = 0x03EF;   // голубой

/* ================= STATUS BAR ================= */

constexpr uint16_t C_STATUS_WIFI_OK    = 0x07E0;
constexpr uint16_t C_STATUS_WIFI_WAIT  = 0xFFE0;
constexpr uint16_t C_STATUS_WIFI_ERR   = 0xF800;

constexpr uint16_t C_STATUS_NTP_OK     = 0x07E0;
constexpr uint16_t C_STATUS_NTP_WAIT   = 0xFFE0;
constexpr uint16_t C_STATUS_NTP_ERR    = 0xF800;

/* ================= ОТЛАДКА ================= */

constexpr uint16_t C_DEBUG_1 = 0xF81F; // пурпур
constexpr uint16_t C_DEBUG_2 = 0x07FF; // циан
constexpr uint16_t C_DEBUG_3 = 0xFFE0; // жёлтый