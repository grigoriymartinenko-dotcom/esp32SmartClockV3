#include <Arduino.h>
#include <WiFi.h>

// ================= CONFIG =================
#include "config/Pins.h"

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
#include "services/RtcTimeProvider.h"
#include "services/NtpTimeProvider.h"
#include "services/NightService.h"
#include "services/ForecastService.h"
#include "services/DhtService.h"
#include "services/ConnectivityService.h"
#include "services/RtcService.h"
#include "services/PreferencesService.h"
#include "services/WifiService.h"
#include "services/NightTransitionService.h"
#include "services/ColorTemperatureService.h"
#include "services/BrightnessService.h"
#include "services/BacklightService.h"

// ================= LAYOUT =================
#include "services/LayoutService.h"

// ================= UI =================
#include "ui/StatusBar.h"
#include "ui/UiSeparator.h"

// ================= SCREENS =================
#include "screens/ClockScreen.h"
#include "screens/ForecastScreen.h"
#include "screens/SettingsScreen.h"

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);
DhtService dht(DHT_PIN, DHT_TYPE);

Buttons buttons(
    BTN_LEFT, BTN_RIGHT, BTN_OK, BTN_BACK,
    50, 800
);

LayoutService layout(tft);
UiVersionService uiVersion;
NightTransitionService nightTransition;
ThemeService themeService(uiVersion);

PreferencesService prefs;
NightService nightService(uiVersion, prefs);

ColorTemperatureService colorTemp;
BrightnessService brightness;
BacklightService backlight;

WifiService wifi(uiVersion, prefs);

ForecastService forecastService(
    "07108cf067a5fdf5aa26dce75354400f",
    "Kharkiv",
    "metric",
    "en"
);

RtcService rtc(RTC_CLK, RTC_DAT, RTC_RST);

RtcTimeProvider rtcProvider(rtc);
NtpTimeProvider ntpProvider;

TimeService timeService(uiVersion);

StatusBar statusBar(
    tft,
    themeService,
    timeService,
    wifi
);

ButtonBar buttonBar(tft, themeService, layout);

ConnectivityService connectivity(timeService);

UiSeparator sepStatus(tft, themeService, layout);
UiSeparator sepBottom(tft, themeService, layout);

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
    uiVersion
);

SettingsScreen settingsScreen(
    tft,
    themeService,
    layout,
    nightService,
    timeService,
    wifi,
    brightness,
    uiVersion,
    buttonBar
);

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
    Serial.println("BOOT");

    uiVersion.begin();
    prefs.begin();
    nightService.begin();

    timeService.setTimezone(
        prefs.tzGmtOffset(),
        prefs.tzDstOffset()
    );

    timeService.registerProvider(rtcProvider);
    timeService.registerProvider(ntpProvider);

    backlight.begin();
    backlight.set(1.0f);

    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(0x0000);

    brightness.begin();
    buttons.begin();
    themeService.begin();
    rtc.begin();
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

    // 1️⃣ INPUT — всегда первым
    ButtonEvent e;
    while (buttons.poll(e)) {
        app.handleEvent(e);
    }

    // 2️⃣ ВАЖНЫЕ сервисы
    timeService.update();
    wifi.update();
    connectivity.update();

    // 3️⃣ UI
    nightService.update(timeService);
    themeService.setNight(nightService.isNight());
    nightTransition.setTarget(nightService.isNight());
    nightTransition.update();

    colorTemp.set(
        nightTransition.value() > 0.7f ? ColorTemp::NIGHT :
        nightTransition.value() > 0.3f ? ColorTemp::EVENING :
                                         ColorTemp::DAY
    );

    screenManager.update();

    // 4️⃣ Медленные
    dht.update();
    forecastService.update();

    // RTC sync
    if (timeService.shouldWriteRtc()) {
        tm now;
        if (getLocalTime(&now)) {
            rtc.write(now);
            timeService.markRtcWritten();
        }
    }
}