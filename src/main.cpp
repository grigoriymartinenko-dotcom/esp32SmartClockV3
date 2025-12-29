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
#include "services/ThemeBlend.h"
#include "services/TimeService.h"
#include "services/NightService.h"
#include "services/ForecastService.h"
#include "services/DhtService.h"
#include "services/ConnectivityService.h"
#include "services/RtcService.h"
#include "services/PreferencesService.h"
#include "services/WifiService.h"
#include "services/NightTransitionService.h"
#include "services/ColorTemperatureService.h"

// ================= LAYOUT =================
#include "services/LayoutService.h"

// ================= UI =================
#include "ui/StatusBar.h"
#include "ui/UiSeparator.h"

// ================= SCREENS =================
#include "screens/ClockScreen.h"
#include "screens/ForecastScreen.h"
#include "screens/SettingsScreen.h"
#include "services/BrightnessService.h"

// =====================================================
// PINOUT
// =====================================================

// ===== TFT (SPI) =====
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4
#define TFT_BL 12
#define TFT_BL_CH  0   // PWM канал для подсветки
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
    50,
    800
);

// =====================================================
// LAYOUT
// =====================================================
LayoutService layout(tft);

// =====================================================
// UI VERSION
// =====================================================
UiVersionService uiVersion;
NightTransitionService nightTransition;

// =====================================================
// SERVICES
// =====================================================
ThemeService themeService(uiVersion);
TimeService  timeService(uiVersion);

PreferencesService prefs;
NightService nightService(uiVersion, prefs);

// ===== COLOR TEMPERATURE =====
ColorTemperatureService colorTemp;

// ===== WIFI =====
WifiService wifi(uiVersion, prefs);

ForecastService forecastService(
    "07108cf067a5fdf5aa26dce75354400f",
    "Kharkiv",
    "metric",
    "en"
);

RtcService rtc(RTC_CLK, RTC_DAT, RTC_RST);

// =====================================================
// UI ELEMENTS
// =====================================================
StatusBar statusBar(
    tft,
    themeService,
    nightTransition,
    colorTemp,        // ← НОВОЕ
    timeService,
    wifi
);

ButtonBar buttonBar(
    tft,
    themeService,
    layout
);

// =====================================================
// CONNECTIVITY (БЕЗ UI)
// =====================================================
ConnectivityService connectivity(timeService);

// =====================================================
// SEPARATORS
// =====================================================
UiSeparator sepStatus(tft, themeService, layout);
UiSeparator sepBottom(tft, themeService, layout);

// =====================================================
// SCREENS
// =====================================================
ClockScreen clockScreen(
    tft,
    timeService,
    nightTransition,
    themeService,
    layout,
    uiVersion,
    dht
);

ForecastScreen forecastScreen(
    tft,
    themeService,
    forecastService,
    layout,
    uiVersion   // 👈 ДОБАВИТЬ
);

SettingsScreen settingsScreen(
    tft,
    themeService,
    layout,
    nightService,
    timeService,
    wifi,
    uiVersion,
    buttonBar
);

// =====================================================
// SCREEN MANAGER
// =====================================================
ScreenManager screenManager(
    tft,
    clockScreen,
    statusBar,
    buttonBar,
    layout,
    sepStatus,
    sepBottom,
    uiVersion,
    themeService
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
BrightnessService brightness;
// =====================================================
// SETUP
// =====================================================
void setup() {

    Serial.begin(115200);
    delay(500);
    Serial.println("BOOT");

    uiVersion.begin();
    prefs.begin();
    nightService.begin();

    timeService.setTimezone(
        prefs.tzGmtOffset(),
        prefs.tzDstOffset()
    );
// 1. Гарантируем, что подсветка не вспыхнет при старте
pinMode(TFT_BL, OUTPUT);
digitalWrite(TFT_BL, LOW);

tft.initR(INITR_BLACKTAB);
tft.setRotation(1);
tft.fillScreen(0x0000);

ledcSetup(TFT_BL_CH, 5000, 8);
ledcAttachPin(TFT_BL, TFT_BL_CH);

brightness.attach([](uint8_t hw) {
    ledcWrite(TFT_BL_CH, hw);
});

brightness.begin();
brightness.apply();

    buttons.begin();
    themeService.begin();

    rtc.begin();
    tm rtcTime;
    if (rtc.read(rtcTime)) {
        timeService.setFromRtc(rtcTime);
    }

    timeService.begin();
    wifi.begin();
    connectivity.begin();

    layout.begin();
    dht.begin();
    forecastService.begin();

    screenManager.begin();
    app.begin();
}

// =====================================================
// LOOP
// =====================================================
void loop() {

    timeService.update();
    nightService.update(timeService);

    const bool nightNow = nightService.isNight();
    themeService.setNight(nightNow);        // legacy
    nightTransition.setTarget(nightNow);
    nightTransition.update();

    // TEMP AUTO (пока просто)
    colorTemp.set(
        nightTransition.value() > 0.7f
            ? ColorTemp::NIGHT
            : nightTransition.value() > 0.3f
                ? ColorTemp::EVENING
                : ColorTemp::DAY
    );

    wifi.update();
    dht.update();
    connectivity.update();

    ButtonEvent e;
    while (buttons.poll(e)) {
        app.handleEvent(e);
    }

    if (timeService.shouldWriteRtc()) {
        tm now;
        if (getLocalTime(&now)) {
            rtc.write(now);
            timeService.markRtcWritten();
        }
    }

    // =========================================================
    // [CHANGE] СНАЧАЛА РИСУЕМ UI, ПОТОМ УЖЕ БЛОКИРУЮЩИЙ HTTP/JSON
    // =========================================================
    screenManager.update();

    // =========================================================
    // [CHANGE] ForecastService.update() перенесён В КОНЕЦ loop()
    // чтобы не рвать кадр во время отрисовки.
    // =========================================================
    forecastService.update();
}