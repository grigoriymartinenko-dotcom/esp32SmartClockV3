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
#include "services/RtcService.h"
#include "services/PreferencesService.h"

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
// PINOUT
// =====================================================

// ===== TFT (SPI) =====
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

// ===== RTC (DS1302) =====
#define RTC_CLK 14
#define RTC_DAT 27
#define RTC_RST 19

// ===== DHT =====
#define DHT_PIN  13
#define DHT_TYPE DHT11
DhtService dht(DHT_PIN, DHT_TYPE);

// ===== BUTTONS =====
#define BTN_LEFT   16
#define BTN_RIGHT  17
#define BTN_OK     25
#define BTN_BACK   26

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

// RTC service
RtcService rtc(
    RTC_CLK,
    RTC_DAT,
    RTC_RST
);

PreferencesService prefs;

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
    timeService,
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
// RTC sync guard
// =====================================================
static bool rtcWrittenAfterNtp = false;

// =====================================================
// SETUP
// =====================================================
void setup() {
    Serial.begin(115200);

    uiVersion.begin();

    // ---------- Preferences ----------
    prefs.begin();

    // ---------- Night ----------
    nightService.begin();

    NightModePref nm = prefs.nightMode();
    nightService.setMode(
        nm == NightModePref::AUTO ? NightService::Mode::AUTO :
        nm == NightModePref::ON   ? NightService::Mode::ON   :
                                    NightService::Mode::OFF
    );
    nightService.setAutoRange(
        prefs.nightStart(),
        prefs.nightEnd()
    );

    // ---------- Timezone ----------
    timeService.setTimezone(
        prefs.tzGmtOffset(),
        prefs.tzDstOffset()
    );

    // ---------- TFT ----------
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(0x0000);

    // ---------- Buttons ----------
    pinMode(BTN_LEFT,  INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_OK,    INPUT_PULLUP);
    pinMode(BTN_BACK,  INPUT_PULLUP);
    buttons.begin();

    themeService.begin();

    // ---------- RTC ----------
    rtc.begin();
    tm rtcTime;
    if (rtc.read(rtcTime)) {
        timeService.setFromRtc(rtcTime);
    }

    // ---------- NTP ----------
    timeService.begin();

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

    // =================================================
    // 1) БЫСТРЫЕ сервисы (НЕ блокируют UI)
    // =================================================
    timeService.update();
    nightService.update(timeService);
    dht.update();
    connectivity.update();

    // =================================================
    // 2) INPUT — ВСЕГДА ПЕРВЫМ
    //    (кнопки и навигация не должны ждать HTTP)
    // =================================================
    ButtonEvent e;
    while (buttons.poll(e)) {
        app.handleEvent(e);
    }

    // =================================================
    // 3) МЕДЛЕННЫЕ сервисы
    //    forecastService.update() внутри делает
    //    HTTP + TLS + JSON → может блокировать
    // =================================================
    forecastService.update();

    // =================================================
    // 4) RTC write-back после NTP (редко)
    // =================================================
    if (!rtcWrittenAfterNtp &&
        timeService.syncState() == TimeService::SYNCED) {

        tm now;
        if (getLocalTime(&now)) {
            rtc.write(now);
            rtcWrittenAfterNtp = true;
        }
    }

    // =================================================
    // 5) DRAW (централизованно)
    // =================================================
    screenManager.update();
}