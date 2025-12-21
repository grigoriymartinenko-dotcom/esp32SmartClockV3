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
// DS1302: CLK/DAT/RST — это НЕ I2C, НЕ Wire, обычные GPIO
#define RTC_CLK 14
#define RTC_DAT 27
#define RTC_RST 19

// ===== DHT =====
#define DHT_PIN  13
#define DHT_TYPE DHT11
DhtService dht(DHT_PIN, DHT_TYPE);

// ===== BUTTONS =====
// ⚠️ ВАЖНО ПРО ESP32:
// GPIO 34/35/36/39 = input-only и БЕЗ внутренних pullup/pulldown.
// Поэтому "кнопки без внешних 10kΩ" на этих пинах НЕВОЗМОЖНЫ.
// Если кнопки реально без резисторов — перенеси их на другие GPIO.
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

// RTC service (DS1302)
RtcService rtc(
    RTC_CLK,
    RTC_DAT,
    RTC_RST
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
// Мы пишем время в RTC один раз после успешной NTP синхронизации.
static bool rtcWrittenAfterNtp = false;

// =====================================================
// SETUP
// =====================================================
void setup() {
    Serial.begin(115200);

    // ---------- UI versions ----------
    uiVersion.begin();

    // ---------- TFT ----------
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(0x0000);

pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
    buttons.begin();

    // ---------- Theme ----------
    themeService.begin();

    // ---------- RTC ----------
    // Сначала стартуем RTC и пробуем взять время.
    rtc.begin();

    tm rtcTime;
    if (rtc.read(rtcTime)) {
        // Если RTC валидный — сразу задаём время в TimeService.
        timeService.setFromRtc(rtcTime);
    }

    // ---------- Time (NTP) ----------
    // Потом запускаем NTP. Он обновит время позже (не блокируя).
    timeService.setTimezone(2 * 3600, 3600);
    timeService.begin();

    // ---------- Night ----------
    nightService.begin();

    // ---------- Layout / Sensors ----------
    layout.begin();
    dht.begin();

    // ---------- Wi-Fi ----------
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    connectivity.begin();

    // ---------- Forecast ----------
    forecastService.begin();

    // ---------- UI core ----------
    screenManager.begin();
    app.begin();
}

// =====================================================
// LOOP
// =====================================================
void loop() {

    // ---------- Services ----------
    timeService.update();

    // ---------- RTC write-back after NTP ----------
    // Как только NTP успешно синхронизировался — запишем время в RTC (один раз).
    if (!rtcWrittenAfterNtp && timeService.syncState() == TimeService::SYNCED) {
        tm now;
        if (getLocalTime(&now)) {
            rtc.write(now);
            rtcWrittenAfterNtp = true;
        }
    }

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