#pragma once
#include <Arduino.h>
#include <Wire.h>

// =====================================================
// Night mode
// =====================================================
enum class NightModePref : uint8_t {
    AUTO = 0,
    ON   = 1,
    OFF  = 2
};

// =====================================================
// Time source preference
// =====================================================
enum class TimeSourcePref : uint8_t {
    AUTO        = 0,
    RTC_ONLY   = 1,
    NTP_ONLY   = 2,
    LOCAL_ONLY = 3
};

// ⚠️ packed — без padding
struct __attribute__((packed)) PreferencesData {
    uint8_t  version;

    // ===== Night =====
    uint8_t  nightMode;
    uint16_t nightStart;
    uint16_t nightEnd;

    // ===== Time =====
    uint8_t  timeSource;

    // ===== Timezone =====
    int32_t  tzGmtOffset;
    int32_t  tzDstOffset;

    // ===== Wi-Fi =====
    uint8_t  wifiEnabled;

    char     wifiSsid[32];
    char     wifiPass[32];
    uint8_t  wifiSaved;

    // ===== Other =====
    uint8_t  brightness;
    uint8_t  lastScreen;

    uint8_t  crc;
};

class PreferencesService {
public:
    explicit PreferencesService(uint8_t addr = 0x50);

    void begin();

    // ===== Wi-Fi =====
    bool wifiEnabled() const;
    void setWifiEnabled(bool on);

    bool hasWifiCredentials() const;
    const char* wifiSsid() const;
    const char* wifiPass() const;

    void setWifiCredentials(const char* ssid, const char* pass);
    void clearWifiCredentials();

    // ===== Timezone =====
    int32_t tzGmtOffset() const;      // 🔥 ВОЗВРАЩЕНО
    int32_t tzDstOffset() const;      // 🔥 ВОЗВРАЩЕНО
    void setTimezone(int32_t gmt, int32_t dst);

    // ===== Save =====
    void save();
    void resetToDefaults();

private:
    uint8_t eepromAddr;
    PreferencesData data{};
    PreferencesData lastSaved{};

    void load();
    void applyDefaults();
    bool isValid(const PreferencesData& d) const;

    void writeBlock(const uint8_t* buf, uint16_t len);
    void readBlock(uint8_t* buf, uint16_t len);

    static uint8_t calcCrc(const PreferencesData& d);
};