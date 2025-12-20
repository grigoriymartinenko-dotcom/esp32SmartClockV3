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
#include "services/DhtService.h"

// ================= LAYOUT =================
#include "services/LayoutService.h"

// ================= UI =================
#include "ui/StatusBar.h"
#include "ui/BottomBar.h"
#include "ui/UiSeparator.h"

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
// DHT
// =====================================================
#define DHT_PIN  25
#define DHT_TYPE DHT11
DhtService dht(DHT_PIN, DHT_TYPE);

// =====================================================
// BUTTONS
// =====================================================
#define BTN_FORECAST 22
#define BTN_CLOCK    21

bool lastForecastBtn = HIGH;
bool lastClockBtn    = HIGH;

uint32_t lastBtnMs = 0;
const uint32_t BTN_DEBOUNCE_MS = 200;

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
// LAYOUT
// =====================================================
LayoutService layout(tft);

// =====================================================
// UI BARS
// =====================================================
StatusBar statusBar(
    tft,
    themeService,
    timeService
);

BottomBar bottomBar(
    tft,
    themeService,
    layout,
    dht
);

// =====================================================
// SEPARATORS
// =====================================================
UiSeparator sepStatus(tft, themeService, 0);
UiSeparator sepBottom(tft, themeService, 0);

// =====================================================
// SCREENS
// =====================================================
ClockScreen clockScreen(
    tft,
    timeService,
    nightService,
    themeService,
    layout
);

ForecastScreen forecastScreen(
    tft,
    themeService,
    forecastService,
    layout
);

// =====================================================
// SCREEN MANAGER
// =====================================================
ScreenManager screenManager(
    clockScreen,
    statusBar,
    bottomBar
);

// =====================================================
// SETUP
// =====================================================
void setup() {
    Serial.begin(115200);

    // ---------- TFT ----------
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(0x0000);

    // ---------- Buttons ----------
    pinMode(BTN_FORECAST, INPUT_PULLUP);
    pinMode(BTN_CLOCK,    INPUT_PULLUP);

    // ---------- Theme ----------
    themeService.begin();

    // ---------- Time ----------
    timeService.setTimezone(2 * 3600, 3600); // Украина
    timeService.begin();

    // ---------- Layout ----------
    layout.begin();

    // ---------- DHT ----------
    dht.begin();

    sepStatus.setY(layout.sepStatusY());
    sepBottom.setY(layout.sepBottomY());

    // ---------- Wi-Fi ----------
    Serial.println("[WiFi] Connecting...");
    statusBar.setWiFiStatus(StatusBar::CONNECTING);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[WiFi] Connected");

    statusBar.setWiFiStatus(StatusBar::ONLINE);

    // ---------- Forecast ----------
    forecastService.begin();

    // ---------- Screens ----------
    screenManager.begin();
}

// =====================================================
// LOOP
// =====================================================
void loop() {

    // ---------- Services ----------
    timeService.update();
    forecastService.update();
    dht.update();

    // ---------- NTP → StatusBar ----------
    static TimeService::SyncState lastNtpState = TimeService::NOT_STARTED;
    auto st = timeService.syncState();

    if (st != lastNtpState) {
        if (st == TimeService::SYNCED) {
            statusBar.setNtpStatus(StatusBar::ONLINE);
        } else if (st == TimeService::SYNCING) {
            statusBar.setNtpStatus(StatusBar::CONNECTING);
        } else {
            statusBar.setNtpStatus(StatusBar::OFFLINE);
        }
        lastNtpState = st;
    }
if (timeService.syncState() == TimeService::ERROR) {
    statusBar.setNtpStatus(StatusBar::ERROR);
}
    // ---------- BottomBar refresh ----------
    static uint32_t lastUiMs = 0;
    if (millis() - lastUiMs > 3000) {
        lastUiMs = millis();
        bottomBar.markDirty();
    }

    // ---------- Buttons ----------
    bool forecastBtn = digitalRead(BTN_FORECAST);
    bool clockBtn    = digitalRead(BTN_CLOCK);
    uint32_t nowMs = millis();

    if (lastForecastBtn == HIGH && forecastBtn == LOW) {
        if (nowMs - lastBtnMs > BTN_DEBOUNCE_MS) {
            screenManager.set(forecastScreen);
            sepStatus.markDirty();
            sepBottom.markDirty();
            lastBtnMs = nowMs;
        }
    }

    if (lastClockBtn == HIGH && clockBtn == LOW) {
        if (nowMs - lastBtnMs > BTN_DEBOUNCE_MS) {
            screenManager.set(clockScreen);
            sepStatus.markDirty();
            sepBottom.markDirty();
            lastBtnMs = nowMs;
        }
    }

    lastForecastBtn = forecastBtn;
    lastClockBtn    = clockBtn;

    // ---------- Draw order ----------
    screenManager.update();

    if (screenManager.currentHasStatusBar()) {
        statusBar.draw();
    }
    if (screenManager.currentHasBottomBar()) {
        bottomBar.draw();
    }
    sepStatus.draw();
    sepBottom.draw();
}