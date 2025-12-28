#pragma once
/*
 * ForecastModel.h
 * ----------------
 * Модель данных прогноза погоды.
 * ❌ НЕТ логики сети
 * ❌ НЕТ UI
 * ✅ ТОЛЬКО структура данных
 */

#include <Arduino.h>
#include <math.h>

// максимум дней, которые мы храним (FREE API = до 5)
static constexpr uint8_t FORECAST_MAX_DAYS = 5;

// ------------------------------------------------------------
// Данные одного дня прогноза
// ------------------------------------------------------------
struct ForecastDay {
    uint32_t dt = 0;        // unix time (начало дня, local)
    uint8_t  weekday = 0;  // 0..6 (Sun..Sat)

    float tempDay   = NAN;
    float tempNight = NAN;

    uint8_t humidity = 0;

    // --------------------------------------------------------
    // Тип погоды (OpenWeather)
    // --------------------------------------------------------
    uint16_t weatherCode = 0; // weather[0].id (200..804)
};;

// ------------------------------------------------------------
// Полная модель прогноза
// ------------------------------------------------------------
struct ForecastModel {
    ForecastDay days[FORECAST_MAX_DAYS];
    uint8_t daysCount = 0;

    bool ready = false;
    uint32_t updatedAtMs = 0;

    char lastError[64] = {0};

    // --------------------------------------------------------
    // Сброс модели
    // --------------------------------------------------------
    void reset() {
        daysCount = 0;
        ready = false;
        updatedAtMs = 0;
        lastError[0] = '\0';

        for (uint8_t i = 0; i < FORECAST_MAX_DAYS; i++) {
            days[i] = ForecastDay();
        }
    }

    // --------------------------------------------------------
    // Сегодняшний день (0-й)
    // --------------------------------------------------------
    const ForecastDay* today() const {
        if (!ready || daysCount == 0) return nullptr;
        return &days[0];
    }

    // --------------------------------------------------------
    // Произвольный день (0..daysCount-1)
    // --------------------------------------------------------
    const ForecastDay* day(uint8_t index) const {
        if (!ready || index >= daysCount) return nullptr;
        return &days[index];
    }
};