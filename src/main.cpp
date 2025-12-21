#include <Arduino.h>
#include <WiFi.h>

// ================= TFT =================
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// ================= CORE =================
#include "core/ScreenManager.h"
#include "core/AppController.h"

// ================= INPUT =================
#include "input/Buttons.h"

// ================= SERVICES =================
#include "services/UiVersionService.h"
#include "services/ThemeService.h"
#include "services/TimeService.h"
#include "services/NightService.h"
#include "services/ForecastService.h"
#include "services/DhtService.h"
#include "services/ConnectivityService.h"

// ================= LAYOUT =================
#include "services/LayoutService.h"

// ================= UI =================
#include "ui/StatusBar.h"
#include "ui/BottomBar.h"
#include "ui/UiSeparator.h"

// ================= SCREENS =================
#include "screens/ClockScreen.h"
#include "screens/ForecastScreen.h"
#include "screens/SettingsScreen.h"

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
#define BTN_LEFT   17
#define BTN_RIGHT  16
#define BTN_OK     22
#define BTN_BACK   21

Buttons buttons(
    BTN_LEFT,
    BTN_RIGHT,
    BTN_OK,
    BTN_BACK,
    50,   // debounce ms
    800   // long press ms
);

// =====================================================
// Wi-Fi / Weather
// =====================================================
static const char* WIFI_SSID = "grig";
static const char* WIFI_PASS = "magnetic";

static const char* OPENWEATHER_KEY = "07108cf067a5fdf5aa26dce75354400f";
static const char* CITY  = "Kharkiv";
static const char* UNITS = "metric";
static const char* LANG  = "en";

// =====================================================
// UI VERSION
// =====================================================
UiVersionService uiVersion;

// =====================================================
// SERVICES
// =====================================================
ThemeService themeService(uiVersion);
TimeService  timeService(uiVersion);
NightService nightService(uiVersion);

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
// UI ELEMENTS
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
// CONNECTIVITY
// =====================================================
ConnectivityService connectivity(
    statusBar,
    timeService
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
    layout,
    uiVersion
);

ForecastScreen forecastScreen(
    tft,
    themeService,
    forecastService,
    layout
);

SettingsScreen settingsScreen(
    tft,
    themeService,
    layout,
    nightService,
    uiVersion
);

// =====================================================
// SCREEN MANAGER
// =====================================================
ScreenManager screenManager(
    tft,
    clockScreen,
    statusBar,
    bottomBar,
    layout,
    sepStatus,
    sepBottom,
    uiVersion
);

// =====================================================
// APP CONTROLLER
// =====================================================
AppController app(
    screenManager,
    clockScreen,
    forecastScreen,
    settingsScreen
);

// =====================================================
// SETUP
// =====================================================
void setup() {
    Serial.begin(115200);

    uiVersion.begin();

    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(0x0000);

    buttons.begin();

    themeService.begin();

    timeService.setTimezone(2 * 3600, 3600);
    timeService.begin();

    nightService.begin();

    layout.begin();
    dht.begin();

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    connectivity.begin();

    forecastService.begin();

    screenManager.begin();
    app.begin();
}

// =====================================================
// LOOP
// =====================================================
void loop() {

    // ---------- Services ----------
    timeService.update();
    nightService.update(timeService);
    forecastService.update();
    dht.update();
    connectivity.update();

    // ---------- Input (events) ----------
    ButtonEvent e;
    while (buttons.poll(e)) {
        app.handleEvent(e);
    }

    // ---------- Draw ----------
    screenManager.update();

    if (screenManager.currentHasStatusBar()) {
        statusBar.update();
    }

    if (screenManager.currentHasBottomBar()) {
        bottomBar.update();
    }

    sepStatus.update();
    sepBottom.update();
}