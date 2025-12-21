#include "services/PreferencesService.h"

static constexpr uint16_t EEPROM_BASE = 0x0000;
static constexpr uint8_t  PREF_VERSION = 4; // ⬅️ bump

PreferencesService::PreferencesService(uint8_t addr)
    : eepromAddr(addr)
{}

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
    if (d.version != PREF_VERSION) return false;
    return d.crc == calcCrc(d);
}

void PreferencesService::applyDefaults() {
    data.version     = PREF_VERSION;

    // Night
    data.nightMode   = static_cast<uint8_t>(NightModePref::AUTO);
    data.nightStart  = 22 * 60;
    data.nightEnd    = 6 * 60;

    // Timezone (Kyiv default)
    data.tzGmtOffset = 2 * 3600;
    data.tzDstOffset = 3600;

    data.brightness  = 80;
    data.lastScreen  = 0;

    data.crc         = calcCrc(data);
}

void PreferencesService::resetToDefaults() {
    applyDefaults();
    save();
}

void PreferencesService::save() {
    data.crc = calcCrc(data);

    if (memcmp(&data, &lastSaved, sizeof(data)) == 0)
        return;

    writeBlock(reinterpret_cast<uint8_t*>(&data), sizeof(data));
    lastSaved = data;
}

// ===== getters =====

NightModePref PreferencesService::nightMode() const {
    return static_cast<NightModePref>(data.nightMode);
}

uint16_t PreferencesService::nightStart() const {
    return data.nightStart;
}

uint16_t PreferencesService::nightEnd() const {
    return data.nightEnd;
}

int32_t PreferencesService::tzGmtOffset() const {
    return data.tzGmtOffset;
}

int32_t PreferencesService::tzDstOffset() const {
    return data.tzDstOffset;
}

// ===== setters =====

void PreferencesService::setNightMode(NightModePref m) {
    data.nightMode = static_cast<uint8_t>(m);
}

void PreferencesService::setNightRange(uint16_t startMin, uint16_t endMin) {
    data.nightStart = constrain(startMin, 0, 1439);
    data.nightEnd   = constrain(endMin,   0, 1439);
}

void PreferencesService::setTimezone(int32_t gmtOffset, int32_t dstOffset) {
    data.tzGmtOffset = gmtOffset;
    data.tzDstOffset = dstOffset;
}

// ===== EEPROM low-level =====

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