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
 * АРХИТЕКТУРА:
 *  - HTTP + JSON выполняются в отдельной FreeRTOS задаче
 *  - update() НЕ блокирует
 *  - UI и кнопки всегда живые
 *
 * ВАЖНЫЕ ФИКСЫ (сохранены из твоей версии):
 *  1) weatherCode всегда инициализируется (иконки не пропадают)
 *  2) tempDay/tempNight могут быть NAN — это допустимые данные
 *  3) локальный день через (tm_year, tm_yday)
 */

// ============================================================================
// ctor
// ============================================================================
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
{
}

// ============================================================================
// begin
// ============================================================================
void ForecastService::begin() {

    _model.reset();

    _lastAttemptMs = 0;
    _lastUpdateMs  = 0;

    _updating   = false;
    _needUpdate = false;

    setError("");

    // ------------------------------------------------------------
    // Запускаем отдельную задачу для HTTP + JSON
    // ------------------------------------------------------------
    xTaskCreatePinnedToCore(
        taskEntry,
        "ForecastTask",
        8192,           // ОБЯЗАТЕЛЬНО: JSON большой
        this,
        1,              // низкий приоритет
        &_task,
        1               // второй core (UI на первом)
    );
}

// ============================================================================
// update (НЕ БЛОКИРУЕТ)
// ============================================================================
void ForecastService::update() {

    const uint32_t now = millis();

    // Не спамим попытками
    if (now - _lastAttemptMs < RETRY_INTERVAL_MS)
        return;

    if (!shouldUpdate())
        return;

    if (_needUpdate || _updating)
        return;

    _lastAttemptMs = now;

    if (WiFi.status() != WL_CONNECTED) {
        setError("WiFi not connected");
        return;
    }

    // Просим задачу обновиться
    _needUpdate = true;
}

// ============================================================================
// task entry
// ============================================================================
void ForecastService::taskEntry(void* arg) {
    static_cast<ForecastService*>(arg)->taskLoop();
}

// ============================================================================
// task loop (ЗДЕСЬ МОЖНО БЛОКИРОВАТЬ)
// ============================================================================
void ForecastService::taskLoop() {

    for (;;) {

        if (_needUpdate) {

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
            _needUpdate = false;
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

// ============================================================================
// state helpers
// ============================================================================
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

// ============================================================================
// URL builder
// ============================================================================
String ForecastService::buildForecastUrl() const {
    String url = FORECAST_URL;
    url += "?q=" + String(_city);
    url += "&units=" + String(_units);
    url += "&lang=" + String(_lang);
    url += "&appid=" + String(_apiKey);
    return url;
}

// ============================================================================
// fetchForecast (БЛОКИРУЮЩАЯ, ТОЛЬКО В ЗАДАЧЕ)
// ============================================================================
bool ForecastService::fetchForecast() {

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    const String url = buildForecastUrl();
    Serial.println(url);

    if (!http.begin(client, url)) {
        setError("HTTP begin failed");
        return false;
    }

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

    // ------------------------------------------------------------------------
    // todayKey (локальный день)
    // ------------------------------------------------------------------------
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

        bool hasCode = false;
        int  weatherCode = 0;
    };

    Acc acc[FORECAST_MAX_DAYS] = {};

    // ------------------------------------------------------------------------
    // Проходим все 3h точки прогноза
    // ------------------------------------------------------------------------
    for (JsonObject item : list) {

        time_t ts = item["dt"].as<time_t>();
        tm t{};
        localtime_r(&ts, &t);

        const int key = (t.tm_year * 400) + t.tm_yday;
        const int dayIndex = key - todayKey;

        if (dayIndex < 0 || dayIndex >= FORECAST_MAX_DAYS)
            continue;

        const float temp = item["main"]["temp"] | NAN;
        const uint8_t hum = item["main"]["humidity"] | 0;

        int wcode = 0;
        JsonArray wArr = item["weather"];
        if (!wArr.isNull() && wArr.size() > 0) {
            wcode = wArr[0]["id"] | 0;
        }

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

    // ------------------------------------------------------------------------
    // Формируем модель
    // ------------------------------------------------------------------------
    for (uint8_t i = 0; i < FORECAST_MAX_DAYS; i++) {

        if (!acc[i].used)
            continue;

        ForecastDay& d = _model.days[_model.daysCount];

        d.dt = acc[i].dayMidnightDt;
        d.weekday = acc[i].weekday;

        d.tempDay   = acc[i].dayCnt   ? (acc[i].daySum   / acc[i].dayCnt)   : NAN;
        d.tempNight = acc[i].nightCnt ? (acc[i].nightSum / acc[i].nightCnt) : NAN;
        d.humidity  = acc[i].hum;

        d.weatherCode = acc[i].hasCode ? acc[i].weatherCode : 800;

        _model.daysCount++;
        if (_model.daysCount >= FORECAST_MAX_DAYS)
            break;
    }

    return _model.daysCount > 0;
}

// ============================================================================
// error
// ============================================================================
void ForecastService::setError(const char* msg) {
    strncpy(_model.lastError, msg, sizeof(_model.lastError) - 1);
    _model.lastError[sizeof(_model.lastError) - 1] = '\0';
}