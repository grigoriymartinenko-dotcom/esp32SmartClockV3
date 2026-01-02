#pragma once

/*
 * Pins.h
 * ------
 * ЕДИНЫЙ pinout проекта.
 *
 * Этот файл подключается:
 *  - в main.cpp
 *  - в hardware-сервисы (BacklightService и т.п.)
 *
 * НЕ содержит логики.
 * ТОЛЬКО определения пинов.
 */
// =====================================================
// PINOUT
// =====================================================

// ===== TFT (SPI) =====
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

// ===== TFT BACKLIGHT =====
// ⚠️ ВАЖНО:
// Если подсветка реально сидит на 3.3V — PWM работать не будет,
// пока BL не будет подключён к GPIO (обычно через транзистор).
#define TFT_BL   12

// ===== RTC (DS1302) =====
#define RTC_CLK 14
#define RTC_DAT 19
#define RTC_RST 27

// ===== DHT =====
#define DHT_PIN  13
#define DHT_TYPE DHT11

// ===== BUTTONS =====
#define BTN_LEFT   16
#define BTN_RIGHT  17
#define BTN_OK     25
#define BTN_BACK   26