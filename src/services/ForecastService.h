#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "models/ForecastModel.h"

/*
 * ============================================================
 * ForecastService (FREE VERSION)
 * ============================================================
 *
 * Использует БЕСПЛАТНЫЙ OpenWeather API 2.5:
 *   /data/2.5/forecast
 *
 * Данные приходят каждые 3 часа → агрегируем по дням.
 *
 * НЕ ИСПОЛЬЗУЕТ One Call 3.0 (платный).
 * ============================================================
 */
class ForecastService {
public:
    ForecastService(
        const char* apiKey,
        const char* city,
        const char* units,
        const char* lang
    );

    void begin();
    void update();

    bool isReady() const;
    const ForecastDay* today() const;
    const char* lastError() const;

private:
    // -------- FREE API --------
    static constexpr const char* FORECAST_URL =
        "https://api.openweathermap.org/data/2.5/forecast";

    static constexpr uint32_t UPDATE_INTERVAL_MS =
        30UL * 60UL * 1000UL;

    static constexpr uint32_t RETRY_INTERVAL_MS =
        10UL * 1000UL;

    const char* _apiKey;
    const char* _city;
    const char* _units;
    const char* _lang;

    ForecastModel _model;

    uint32_t _lastUpdateMs  = 0;
    uint32_t _lastAttemptMs = 0;
    bool _updating = false;

private:
    bool shouldUpdate() const;
    bool fetchForecast();
    String buildForecastUrl() const;
    void setError(const char* msg);
};