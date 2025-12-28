#define ARDUINOJSON_ENABLE_STD_STRING 0
#define ARDUINOJSON_ENABLE_STD_STREAM 0

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>
#include <math.h>

#include "services/ForecastService.h"

/*
 * ForecastService.cpp
 * -------------------
 * FREE OpenWeather /data/2.5/forecast (3h шаг) → агрегируем в дни.
 *
 * ВАЖНОЕ исправление:
 *  - локальный день через (tm_year, tm_yday), а не UTC 86400
 *
 * FIX (по твоим симптомам):
 * 1) Раньше weatherCode вообще не устанавливался -> иконки могли пропадать.
 * 2) tempDay/tempNight могут быть NAN (если нет слотов 09..18 или ночных),
 *    это ОК как данные. Но UI обязан обрабатывать NAN.
 *    (Этим мы займёмся в ForecastScreen.cpp)
 */

ForecastService::ForecastService(
    const char* apiKey,
    const char* city,
    const char* units,
    const char* lang
)
: _apiKey(apiKey)
, _city(city)
, _units(units)
, _lang(lang)
{}

void ForecastService::begin() {
    _model.reset();
    _lastAttemptMs = 0;
    _lastUpdateMs  = 0;
    _updating = false;
    setError("");
}

void ForecastService::update() {
    if (_updating) return;

    const uint32_t now = millis();
    if (now - _lastAttemptMs < RETRY_INTERVAL_MS) return;
    _lastAttemptMs = now;

    if (WiFi.status() != WL_CONNECTED) {
        setError("WiFi not connected");
        return;
    }

    if (!shouldUpdate()) return;

    _updating = true;

    if (fetchForecast()) {
        _lastUpdateMs = millis();
        _model.updatedAtMs = _lastUpdateMs;
        _model.ready = true;
        setError("");
        Serial.printf("[Forecast] OK, days=%d\n", _model.daysCount);
    } else {
        Serial.printf("[Forecast] FAIL: %s\n", lastError());
    }

    _updating = false;
}

bool ForecastService::shouldUpdate() const {
    if (!_model.ready) return true;
    return (millis() - _lastUpdateMs) >= UPDATE_INTERVAL_MS;
}

bool ForecastService::isReady() const {
    return _model.ready;
}

const ForecastDay* ForecastService::today() const {
    return _model.today();
}

const ForecastDay* ForecastService::day(uint8_t index) const {
    return _model.day(index);
}

uint8_t ForecastService::daysCount() const {
    return _model.daysCount;
}

const char* ForecastService::lastError() const {
    return _model.lastError;
}

String ForecastService::buildForecastUrl() const {
    String url = FORECAST_URL;
    url += "?q=" + String(_city);
    url += "&units=" + String(_units);
    url += "&lang=" + String(_lang);
    url += "&appid=" + String(_apiKey);
    return url;
}

bool ForecastService::fetchForecast() {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    const String url = buildForecastUrl();
    Serial.println(url);

    http.begin(client, url);
    const int code = http.GET();

    if (code != HTTP_CODE_OK) {
        setError("HTTP error");
        http.end();
        return false;
    }

    // FREE forecast = большой JSON
    DynamicJsonDocument doc(45000);
    DeserializationError err = deserializeJson(doc, http.getStream());
    http.end();

    if (err) {
        setError(err.c_str());
        return false;
    }

    JsonArray list = doc["list"];
    if (list.isNull()) {
        setError("No list[]");
        return false;
    }

    _model.reset();

    // -----------------------------
    // todayKey (локальный день)
    // -----------------------------
    time_t nowTs = time(nullptr);
    tm nowLocal{};
    localtime_r(&nowTs, &nowLocal);

    const int todayKey = (nowLocal.tm_year * 400) + nowLocal.tm_yday;

    struct Acc {
        bool used = false;

        float daySum = 0;
        float nightSum = 0;
        int dayCnt = 0;
        int nightCnt = 0;

        uint32_t dayMidnightDt = 0;
        uint8_t weekday = 0;

        uint8_t hum = 0;

        // FIX: сохраняем код погоды (для иконок)
        // Берём первое валидное значение для дня.
        bool hasCode = false;
        int  weatherCode = 0;
    };

    Acc acc[FORECAST_MAX_DAYS] = {};

    // -----------------------------
    // Проходим все 3h точки прогноза
    // -----------------------------
    for (JsonObject item : list) {

        time_t ts = item["dt"].as<time_t>();
        tm t{};
        localtime_r(&ts, &t);

        const int key = (t.tm_year * 400) + t.tm_yday;
        const int dayIndex = key - todayKey;

        if (dayIndex < 0 || dayIndex >= FORECAST_MAX_DAYS) {
            continue;
        }

        const float temp = item["main"]["temp"] | NAN;
        const uint8_t hum = item["main"]["humidity"] | 0;

        // FIX: weatherCode для иконки
        // OpenWeather: weather[0].id
        int wcode = 0;
        JsonArray wArr = item["weather"];
        if (!wArr.isNull() && wArr.size() > 0) {
            JsonObject w0 = wArr[0];
            wcode = w0["id"] | 0;
        }

        // "день" = 09..18
        if (t.tm_hour >= 9 && t.tm_hour <= 18) {
            acc[dayIndex].daySum += temp;
            acc[dayIndex].dayCnt++;
        } else {
            acc[dayIndex].nightSum += temp;
            acc[dayIndex].nightCnt++;
        }

        acc[dayIndex].hum = hum;
        acc[dayIndex].used = true;

        if (!acc[dayIndex].hasCode && wcode != 0) {
            acc[dayIndex].hasCode = true;
            acc[dayIndex].weatherCode = wcode;
        }

        if (acc[dayIndex].dayMidnightDt == 0) {
            tm m = t;
            m.tm_hour = 0;
            m.tm_min  = 0;
            m.tm_sec  = 0;
            time_t midnight = mktime(&m);
            acc[dayIndex].dayMidnightDt = (uint32_t)midnight;
            acc[dayIndex].weekday = (uint8_t)m.tm_wday;
        }
    }

    // -----------------------------
    // Собираем модель из acc[] по порядку 0..4
    // -----------------------------
    for (uint8_t i = 0; i < FORECAST_MAX_DAYS; i++) {
        if (!acc[i].used) continue;

        ForecastDay& d = _model.days[_model.daysCount];

        d.dt = acc[i].dayMidnightDt;
        d.weekday = acc[i].weekday;

        d.tempDay   = acc[i].dayCnt   ? (acc[i].daySum   / acc[i].dayCnt)   : NAN;
        d.tempNight = acc[i].nightCnt ? (acc[i].nightSum / acc[i].nightCnt) : NAN;
        d.humidity  = acc[i].hum;

        // FIX: иконки
        d.weatherCode = acc[i].hasCode ? acc[i].weatherCode : 800; // 800 = clear (fallback)

        _model.daysCount++;
        if (_model.daysCount >= FORECAST_MAX_DAYS) break;
    }

    return _model.daysCount > 0;
}

void ForecastService::setError(const char* msg) {
    strncpy(_model.lastError, msg, sizeof(_model.lastError) - 1);
    _model.lastError[sizeof(_model.lastError) - 1] = '\0';
}