#include <Arduino.h>
#include <WiFi.h>

// ================= TFT =================
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// ================= CORE =================
#include "core/ScreenManager.h"

// ================= SERVICES =================
#include "services/UiVersionService.h"
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
// BUTTONS (4 hardware buttons)
// =====================================================
// FIXED ранее: BTN1=GPIO17, BTN2=GPIO16, BTN3=GPIO22, BTN4=GPIO21
#define BTN_LEFT   17
#define BTN_RIGHT  16
#define BTN_OK     22
#define BTN_BACK   21

static const uint32_t BTN_DEBOUNCE_MS = 200;

struct DebouncedButton {
    uint8_t pin = 0;
    bool last = HIGH;
    uint32_t lastMs = 0;

    void begin(uint8_t p) {
        pin = p;
        pinMode(pin, INPUT_PULLUP);
        last = digitalRead(pin);
        lastMs = 0;
    }

    // true = "нажатие" (фронт HIGH->LOW) с debounce
    bool pressed(uint32_t nowMs) {
        bool v = digitalRead(pin);

        bool trig = false;
        if (last == HIGH && v == LOW && (nowMs - lastMs) > BTN_DEBOUNCE_MS) {
            trig = true;
            lastMs = nowMs;
        }

        last = v;
        return trig;
    }
};

DebouncedButton btnLeft;
DebouncedButton btnRight;
DebouncedButton btnOk;
DebouncedButton btnBack;

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
// UI VERSION (v3.2)
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
// UI
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
    layout
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
    sepBottom
);

// =====================================================
// ACTIVE SCREEN (local state for routing buttons)
// =====================================================
enum class ActiveScreen : uint8_t {
    CLOCK = 0,
    FORECAST,
    SETTINGS
};

static ActiveScreen active = ActiveScreen::CLOCK;

static void goClock() {
    screenManager.set(clockScreen);
    active = ActiveScreen::CLOCK;
}

static void goForecast() {
    screenManager.set(forecastScreen);
    active = ActiveScreen::FORECAST;
}

static void goSettings() {
    settingsScreen.clearExitRequest();
    screenManager.set(settingsScreen);
    active = ActiveScreen::SETTINGS;
}

// =====================================================
// SETUP
// =====================================================
void setup() {
    Serial.begin(115200);

    // ---------- UI Versions ----------
    uiVersion.begin();

    // ---------- TFT ----------
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(0x0000);

    // ---------- Buttons ----------
    btnLeft.begin(BTN_LEFT);
    btnRight.begin(BTN_RIGHT);
    btnOk.begin(BTN_OK);
    btnBack.begin(BTN_BACK);

    // ---------- Theme ----------
    themeService.begin();

    // ---------- Time ----------
    timeService.setTimezone(2 * 3600, 3600); // Украина
    timeService.begin();

    // ---------- Layout ----------
    layout.begin();

    // ---------- DHT ----------
    dht.begin();

    // ---------- Wi-Fi ----------
    statusBar.setWiFiStatus(StatusBar::CONNECTING);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
    }
    statusBar.setWiFiStatus(StatusBar::ONLINE);

    // ---------- Forecast ----------
    forecastService.begin();

    // ---------- Screens ----------
    screenManager.begin();
    active = ActiveScreen::CLOCK;
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

    // ==================================================
    // NTP → StatusBar (FIXED LOGIC)
    // ==================================================
    static TimeService::SyncState lastNtpState = TimeService::NOT_STARTED;
    static bool ntpEverSynced = false;

    TimeService::SyncState st = timeService.syncState();

    if (st != lastNtpState) {

        if (st == TimeService::SYNCED) {
            ntpEverSynced = true;
            statusBar.setNtpStatus(StatusBar::ONLINE);
        }
        else if (st == TimeService::SYNCING) {
            statusBar.setNtpStatus(StatusBar::CONNECTING);
        }
        else if (st == TimeService::ERROR) {
            statusBar.setNtpStatus(StatusBar::ERROR);
        }
        else {
            statusBar.setNtpStatus(
                ntpEverSynced ? StatusBar::ONLINE
                              : StatusBar::OFFLINE
            );
        }

        lastNtpState = st;
    }

    // ==================================================
    // Buttons routing
    // ==================================================
    uint32_t now = millis();
// ===== DEBUG BUTTONS (TEMP) =====
static uint32_t lastPrint = 0;
if (now - lastPrint > 300) {
    lastPrint = now;

    Serial.printf(
        "[BTN raw] L=%d R=%d OK=%d BACK=%d\n",
        digitalRead(BTN_LEFT),
        digitalRead(BTN_RIGHT),
        digitalRead(BTN_OK),
        digitalRead(BTN_BACK)
    );
}
    const bool pLeft  = btnLeft.pressed(now);
    const bool pRight = btnRight.pressed(now);
    const bool pOk    = btnOk.pressed(now);
    const bool pBack  = btnBack.pressed(now);
if (pLeft)  Serial.println("[BTN] LEFT pressed");
if (pRight) Serial.println("[BTN] RIGHT pressed");
if (pOk)    Serial.println("[BTN] OK pressed");
if (pBack)  Serial.println("[BTN] BACK pressed");
    if (active == ActiveScreen::SETTINGS) {
        // SETTINGS: навигация внутри экрана
        if (pLeft)  settingsScreen.onLeft();
        if (pRight) settingsScreen.onRight();
        if (pOk)    settingsScreen.onOk();
        if (pBack)  settingsScreen.onBack();

        // выход по флагу (BACK)
        if (settingsScreen.exitRequested()) {
            settingsScreen.clearExitRequest();
            goClock();
        }
    } else {
        // НЕ settings: быстрые действия
        // LEFT  -> Forecast
        // RIGHT -> Clock
        // OK    -> Settings
        // BACK  -> Clock (на всякий)
        if (pLeft)  goForecast();
        if (pRight) goClock();
        if (pOk)    goSettings();
        if (pBack)  goClock();
    }

    // ==================================================
    // Draw order
    // ==================================================
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