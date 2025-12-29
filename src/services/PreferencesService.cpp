#include "services/PreferencesService.h"

static constexpr uint16_t EEPROM_BASE   = 0x0000;
static constexpr uint8_t  PREF_VERSION  = 7;

// ============================================================================
// ctor
// ============================================================================
PreferencesService::PreferencesService(uint8_t addr)
    : eepromAddr(addr)
{}

// ============================================================================
// begin / load
// ============================================================================
void PreferencesService::begin() {
    Wire.begin();
    load();
}

void PreferencesService::load() {
    readBlock(reinterpret_cast<uint8_t*>(&data), sizeof(data));

    if (!isValid(data)) {
        resetToDefaults();
    }

    lastSaved = data;
}

bool PreferencesService::isValid(const PreferencesData& d) const {
    if (d.version != PREF_VERSION)
        return false;
    return d.crc == calcCrc(d);
}

// ============================================================================
// defaults
// ============================================================================
void PreferencesService::applyDefaults() {
    memset(&data, 0, sizeof(data));

    data.version = PREF_VERSION;

    // ===== Night =====
    data.nightMode  = (uint8_t)NightModePref::AUTO;
    data.nightStart = 22 * 60;
    data.nightEnd   = 6 * 60;

    // ===== Timezone =====
    data.tzGmtOffset = 2 * 3600;
    data.tzDstOffset = 3600;

    // ===== Wi-Fi =====
    data.wifiEnabled = 1;
    data.wifiSaved   = 0;

    data.crc = calcCrc(data);
}

void PreferencesService::resetToDefaults() {
    applyDefaults();
    save();
}

// ============================================================================
// save
// ============================================================================
void PreferencesService::save() {
    data.crc = calcCrc(data);

    if (memcmp(&data, &lastSaved, sizeof(data)) == 0)
        return;

    writeBlock(reinterpret_cast<const uint8_t*>(&data), sizeof(data));
    lastSaved = data;
}

// ============================================================================
// NIGHT
// ============================================================================
NightModePref PreferencesService::nightMode() const {
    return (NightModePref)data.nightMode;
}

void PreferencesService::setNightMode(NightModePref mode) {
    data.nightMode = (uint8_t)mode;
}

uint16_t PreferencesService::nightStart() const {
    return data.nightStart;
}

uint16_t PreferencesService::nightEnd() const {
    return data.nightEnd;
}

void PreferencesService::setNightAutoRange(uint16_t startMin, uint16_t endMin) {
    data.nightStart = startMin;
    data.nightEnd   = endMin;
}

// ============================================================================
// WIFI
// ============================================================================
bool PreferencesService::wifiEnabled() const {
    return data.wifiEnabled != 0;
}

void PreferencesService::setWifiEnabled(bool on) {
    data.wifiEnabled = on ? 1 : 0;
}

bool PreferencesService::hasWifiCredentials() const {
    return data.wifiSaved != 0;
}

const char* PreferencesService::wifiSsid() const {
    return data.wifiSsid;
}

const char* PreferencesService::wifiPass() const {
    return data.wifiPass;
}

void PreferencesService::setWifiCredentials(const char* ssid, const char* pass) {
    strncpy(data.wifiSsid, ssid, sizeof(data.wifiSsid) - 1);
    strncpy(data.wifiPass, pass, sizeof(data.wifiPass) - 1);
    data.wifiSaved = 1;
}

void PreferencesService::clearWifiCredentials() {
    data.wifiSaved = 0;
    data.wifiSsid[0] = 0;
    data.wifiPass[0] = 0;
}

// ============================================================================
// TIMEZONE
// ============================================================================
int32_t PreferencesService::tzGmtOffset() const {
    return data.tzGmtOffset;
}

int32_t PreferencesService::tzDstOffset() const {
    return data.tzDstOffset;
}

void PreferencesService::setTimezone(int32_t gmt, int32_t dst) {
    data.tzGmtOffset = gmt;
    data.tzDstOffset = dst;
}

// ============================================================================
// BRIGHTNESS
// ============================================================================
uint8_t PreferencesService::brightness() const {
    return data.brightness;
}

void PreferencesService::setBrightness(uint8_t value) {
    data.brightness = value;
}

// ============================================================================
// EEPROM low-level
// ============================================================================
void PreferencesService::writeBlock(const uint8_t* buf, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        Wire.beginTransmission(eepromAddr);
        Wire.write((EEPROM_BASE + i) >> 8);
        Wire.write((EEPROM_BASE + i) & 0xFF);
        Wire.write(buf[i]);
        Wire.endTransmission();
        delay(5);
    }
}

void PreferencesService::readBlock(uint8_t* buf, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        Wire.beginTransmission(eepromAddr);
        Wire.write((EEPROM_BASE + i) >> 8);
        Wire.write((EEPROM_BASE + i) & 0xFF);
        Wire.endTransmission(false);

        Wire.requestFrom(eepromAddr, (uint8_t)1);
        buf[i] = Wire.available() ? Wire.read() : 0xFF;
    }
}

uint8_t PreferencesService::calcCrc(const PreferencesData& d) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&d);
    uint8_t crc = 0;
    for (size_t i = 0; i < sizeof(PreferencesData) - 1; i++)
        crc ^= p[i];
    return crc;
}