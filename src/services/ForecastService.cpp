#define ARDUINOJSON_ENABLE_STD_STRING 0
#define ARDUINOJSON_ENABLE_STD_STREAM 0

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include "services/ForecastService.h"

/*
 * ============================================================
 * ForecastService.cpp (FREE API 2.5)
 *
 * Получаем прогноз каждые 3 часа и
 * собираем средние значения за день.
 * ============================================================
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

    uint32_t now = millis();
    if (now - _lastAttemptMs < RETRY_INTERVAL_MS)
        return;

    _lastAttemptMs = now;

    if (WiFi.status() != WL_CONNECTED) {
        setError("WiFi not connected");
        return;
    }

    if (!shouldUpdate())
        return;

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
    String url = buildForecastUrl();
    Serial.println(url);

    http.begin(client, url);
    int code = http.GET();

    if (code != HTTP_CODE_OK) {
        setError("HTTP error");
        http.end();
        return false;
    }

    DynamicJsonDocument doc(20000);
    if (deserializeJson(doc, http.getStream())) {
        setError("JSON error");
        http.end();
        return false;
    }
    http.end();

    JsonArray list = doc["list"];
    if (list.isNull()) {
        setError("No list[]");
        return false;
    }

    _model.reset();

    float daySum = 0, nightSum = 0;
    int dayCnt = 0, nightCnt = 0;

    for (JsonObject item : list) {
        int hour = item["dt_txt"].as<String>().substring(11, 13).toInt();
        float temp = item["main"]["temp"] | NAN;

        if (hour >= 9 && hour <= 18) {
            daySum += temp;
            dayCnt++;
        } else {
            nightSum += temp;
            nightCnt++;
        }
    }

    ForecastDay& d = _model.days[0];
    d.tempDay   = dayCnt   ? daySum   / dayCnt   : NAN;
    d.tempNight = nightCnt ? nightSum / nightCnt : NAN;
    d.humidity  = list[0]["main"]["humidity"] | 0;

    _model.daysCount = 1;
    return true;
}

void ForecastService::setError(const char* msg) {
    // ❗ НЕ трогаем ready
    // ready означает: "данные уже когда-то были"
    strncpy(_model.lastError, msg, sizeof(_model.lastError) - 1);
    _model.lastError[sizeof(_model.lastError) - 1] = '\0';
}