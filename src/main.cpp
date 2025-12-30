#include <Arduino.h>
#include <WiFi.h>
// =======================CONFIG==============================
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
#include "services/TimeProvider.h"
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


PreferencesService prefs;
NightService nightService(uiVersion, prefs);
// ===== COLOR TEMPERATURE =====
ColorTemperatureService colorTemp;
// ===== BRIGHTNESS (Variant B) =====
BrightnessService brightness;
// ===== TFT BACKLIGHT (PWM) =====
BacklightService backlight;
// ===== WIFI =====
WifiService wifi(uiVersion, prefs);
ForecastService forecastService(
    "07108cf067a5fdf5aa26dce75354400f",
    "Kharkiv",
    "metric",
    "en"
);

RtcService rtc(RTC_CLK, RTC_DAT, RTC_RST);
// ===== TIME PROVIDERS (async) =====
RtcTimeProvider rtcProvider(rtc);
NtpTimeProvider ntpProvider;

TimeService  timeService(uiVersion);
// =====================================================
// UI ELEMENTS
// =====================================================
StatusBar statusBar(
    tft,
    themeService,
    nightTransition,
    colorTemp,
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
// -------------------------------------------------
// Time providers (priority order)
// -------------------------------------------------
// RTC даёт "примерное" время сразу,
// NTP уточнит позже (асинхронно), если режим AUTO/NTP_ONLY.
timeService.registerProvider(rtcProvider);
timeService.registerProvider(ntpProvider);
    // -------------------------------------------------
    // TFT init + подсветка (PWM)
    // -------------------------------------------------
    // ВАЖНО:
    //  - BacklightService управляет ФИЗИЧЕСКОЙ подсветкой TFT через PWM (LEDC).
    //  - Это НЕ BrightnessService (тот влияет только на яркость ЦВЕТОВ в UI).
    //
    // Если TFT_BL у тебя сейчас реально подключён к 3.3V (как написано в pinout),
    // то PWM работать НЕ будет, пока ты не заведёшь BL на GPIO (обычно через
    // транзистор/MOSFET, либо напрямую, если модуль TFT это допускает).
    //
    // GPIO12 — "strap pin" у ESP32. В целом он может работать, но будь аккуратен
    // с внешними подтяжками на старте. Если будут проблемы с бутом — перенесём BL
    // на другой GPIO.
    backlight.begin();
    backlight.set(0.0f); // не слепим при старте (если BL реально управляем)

    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(0x0000);

    backlight.set(1.0f); // 🔆 подсветка ВКЛ (дальше будет управляться настройками)
    delay(1000);
backlight.set(0.2f);
delay(1000);
backlight.set(1.0f);

    // -------------------------------------------------
    // Services
    // -------------------------------------------------
    brightness.begin();            // пока дефолт 1.0f, позже подтянем prefs

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

    timeService.update();
    nightService.update(timeService);

    const bool nightNow = nightService.isNight();
    themeService.setNight(nightNow);
    nightTransition.setTarget(nightNow);
    nightTransition.update();

    // TEMP AUTO
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

    // UI
    screenManager.update();

    // Network (после UI)
    forecastService.update();
}