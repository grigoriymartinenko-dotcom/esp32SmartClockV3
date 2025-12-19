#include <Arduino.h>
#include <WiFi.h>

// ================= TFT =================
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// ================= CORE =================
#include "core/ScreenManager.h"

// ================= SERVICES =================
#include "services/ThemeService.h"
#include "services/TimeService.h"
#include "services/NightService.h"
#include "services/ForecastService.h"

// ================= UI =================
#include "ui/StatusBar.h"

// ================= SCREENS =================
#include "screens/ClockScreen.h"
#include "screens/ForecastScreen.h"

// =====================================================
// TFT
// =====================================================
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

// =====================================================
// Wi-Fi + OpenWeather
// =====================================================
static const char* WIFI_SSID = "grig";
static const char* WIFI_PASS = "magnetic";

static const char* OPENWEATHER_KEY = "07108cf067a5fdf5aa26dce75354400f";
static const char* CITY  = "Kharkiv";
static const char* UNITS = "metric";
static const char* LANG  = "en";

// =====================================================
// SERVICES
// =====================================================
ThemeService themeService;
TimeService  timeService;
NightService nightService;

ForecastService forecastService(
    OPENWEATHER_KEY,
    CITY,
    UNITS,
    LANG
);

// =====================================================
// UI: STATUS BAR
// =====================================================
StatusBar statusBar(
    tft,
    themeService,
    timeService
);

// =====================================================
// SCREENS
// =====================================================
ClockScreen clockScreen(
    tft,
    timeService,
    nightService,
    themeService
);

ForecastScreen forecastScreen(
    tft,
    themeService,
    forecastService
);

// =====================================================
// SCREEN MANAGER
// =====================================================
ScreenManager screenManager(clockScreen);

// =====================================================
// SETUP
// =====================================================
void setup() {
    Serial.begin(115200);

    // TFT
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(ST7735_BLACK);

    // Theme
    themeService.begin();

    // Time (NTP)
    timeService.begin();

    // Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" OK");

    // Forecast
    forecastService.begin();

    // Screens
    screenManager.begin();
}

// =====================================================
// LOOP
// =====================================================
void loop() {
    // -------- services --------
    timeService.update();
    forecastService.update();

    // -------- screens --------
    screenManager.update();

    // -------- status bar --------
    // Рисуем ТОЛЬКО если экран его поддерживает
    if (screenManager.currentHasStatusBar()) {
        statusBar.setWiFi(WiFi.status() == WL_CONNECTED);
        statusBar.draw();
    }
}