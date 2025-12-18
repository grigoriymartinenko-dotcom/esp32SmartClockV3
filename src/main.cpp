#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_ST7735.h>

#include "core/ScreenManager.h"
#include "screens/ClockScreen.h"

#include "services/TimeService.h"
#include "services/NightService.h"
#include "services/ThemeService.h"

// ================= TFT =================
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

// ================= SERVICES =================
TimeService  timeService;
NightService nightService;
ThemeService themeService;

// ================= SCREEN =================
ClockScreen clockScreen(
    tft,
    timeService,
    nightService,
    themeService.current()
);

ScreenManager screenManager(clockScreen);

// ================= SETUP =================
void setup() {
    Serial.begin(115200);

    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);

    // ===== Wi-Fi =====
    WiFi.begin("grig", "magnetic");
    Serial.print("WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" OK");

    // ===== NTP =====
    configTime(
        2 * 3600,   // GMT+2
        0,
        "pool.ntp.org",
        "time.nist.gov",
        "time.google.com"
    );

    Serial.print("Waiting for NTP");
    time_t now = 0;
    int retries = 0;
    while (now < 8 * 3600 * 2 && retries < 20) {
        delay(500);
        Serial.print(".");
        time(&now);
        retries++;
    }
    Serial.println();

    if (now < 8 * 3600 * 2) {
        Serial.println("❌ NTP FAILED");
    } else {
        Serial.println("✅ NTP OK");
    }

    timeService.begin();
    themeService.begin();

    screenManager.begin();
}

// ================= LOOP =================
void loop() {
    timeService.update();
    nightService.update(timeService.hour());

    screenManager.update();
}