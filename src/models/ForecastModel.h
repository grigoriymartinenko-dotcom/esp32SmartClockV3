#pragma once
/*
 * ForecastModel.h
 * ----------------
 * Модель данных прогноза погоды.
 * Никакой логики, никакого UI.
 */

#include <Arduino.h>

static constexpr uint8_t FORECAST_MAX_DAYS = 8;
static constexpr uint8_t OW_ICON_LEN = 4;
static constexpr uint8_t OW_DESC_LEN = 48;

// ------------------------------------------------------------
// Данные одного дня
// ------------------------------------------------------------
struct ForecastDay {
  uint32_t dt = 0;

  float tempDay   = NAN;
  float tempNight = NAN;

  uint8_t humidity = 0;

  char icon[OW_ICON_LEN] = {0};
  char desc[OW_DESC_LEN] = {0};
};

// ------------------------------------------------------------
// Полная модель прогноза
// ------------------------------------------------------------
struct ForecastModel {
  ForecastDay days[FORECAST_MAX_DAYS];
  uint8_t daysCount = 0;

  bool ready = false;
  uint32_t updatedAtMs = 0;

  char lastError[64] = {0};

  // Сброс модели
  void reset() {
    daysCount = 0;
    ready = false;
    updatedAtMs = 0;
    lastError[0] = '\0';

    for (uint8_t i = 0; i < FORECAST_MAX_DAYS; i++) {
      days[i] = ForecastDay();
    }
  }

  // Сегодняшний день
  const ForecastDay* today() const {
    if (!ready || daysCount == 0) return nullptr;
    return &days[0];
  }
};