#pragma once
#include <Arduino.h>
#include <Wire.h>

enum class NightModePref : uint8_t {
    AUTO = 0,
    ON   = 1,
    OFF  = 2
};

// ⚠️ packed — без padding
struct __attribute__((packed)) PreferencesData {
    uint8_t  version;

    // ===== Night =====
    uint8_t  nightMode;     // NightModePref
    uint16_t nightStart;    // minutes 0..1439
    uint16_t nightEnd;      // minutes 0..1439

    // ===== Timezone =====
    int32_t  tzGmtOffset;   // seconds
    int32_t  tzDstOffset;   // seconds

    // ===== Other =====
    uint8_t  brightness;
    uint8_t  lastScreen;

    uint8_t  crc;
};

class PreferencesService {
public:
    explicit PreferencesService(uint8_t addr = 0x50);

    void begin();

    // ===== Night =====
    NightModePref nightMode() const;
    uint16_t nightStart() const;
    uint16_t nightEnd() const;

    void setNightMode(NightModePref m);
    void setNightRange(uint16_t startMin, uint16_t endMin);

    // ===== Timezone =====
    int32_t tzGmtOffset() const;
    int32_t tzDstOffset() const;
    void setTimezone(int32_t gmtOffset, int32_t dstOffset);

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