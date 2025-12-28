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
#include "services/WifiService.h"
#include "services/NightTransitionService.h"

// ================= LAYOUT =================
#include "services/LayoutService.h"

// ================= UI =================
#include "ui/StatusBar.h"
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
// ===== WIFI =====
WifiService wifi(
    uiVersion,
    prefs
);

ForecastService forecastService(
    "07108cf067a5fdf5aa26dce75354400f",
    "Kharkiv",
    "metric",
    "en"
);

RtcService rtc(
    RTC_CLK,
    RTC_DAT,
    RTC_RST
);

// =====================================================
// UI ELEMENTS
// =====================================================
StatusBar statusBar(
    tft,
    themeService,
    nightTransition,
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
ConnectivityService connectivity(
    timeService
);

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
    nightTransition,   // ✅ ВОТ ОН
    themeService,
    layout,
    uiVersion,
    dht
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

    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(0x0000);

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

    // NightService — единственная "истина": ночь сейчас или день.
    // Он учитывает AUTO/ON/OFF и интервал.
    nightService.update(timeService);

    // ---------------------------------------------------------------------
    // Шаг 5 (NightTransition): делаем переход глобальным (для всех экранов).
    //
    // ВАЖНО:
    //  - themeService.setNight(...) оставляем для совместимости со старыми экранами
    //  - nightTransition.setTarget(...) задаёт ЦЕЛЬ для плавного перехода
    //  - nightTransition.update() крутится ВСЕГДА, независимо от активного экрана
    // ---------------------------------------------------------------------
    const bool nightNow = nightService.isNight();
    themeService.setNight(nightNow);        // жёсткое day/night (legacy)
    nightTransition.setTarget(nightNow);    // цель для плавного коэффициента 0..1
    nightTransition.update();               // переход обновляется каждый loop

    wifi.update();
    dht.update();
    connectivity.update();

    ButtonEvent e;
    while (buttons.poll(e)) {
        app.handleEvent(e);
    }

    forecastService.update();

    if (timeService.shouldWriteRtc()) {
        tm now;
        if (getLocalTime(&now)) {
            rtc.write(now);
            timeService.markRtcWritten();
        }
    }

    screenManager.update();
}