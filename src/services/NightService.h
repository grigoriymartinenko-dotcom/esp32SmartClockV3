#pragma once
#include <stdint.h>

#include "services/UiVersionService.h"
#include "services/TimeService.h"
#include "services/PreferencesService.h"

/*
 * NightService
 * ------------
 * Истина ночного режима.
 *
 * Отвечает за:
 *  - AUTO / ON / OFF
 *  - интервал ночи
 *  - вычисление isNight()
 *
 * НЕ:
 *  - не рисует
 *  - не знает про UI
 *
 * Любая смена состояния → UiChannel::THEME
 */

class NightService {
public:
    enum class Mode : uint8_t {
        AUTO = 0,
        ON,
        OFF
    };

    NightService(
        UiVersionService& uiVersion,
        PreferencesService& prefs
    );

    void begin();

    // основной апдейт
    void update(const TimeService& time);

    // ===== MODE =====
    void setMode(Mode m);
    Mode mode() const;

    // ===== AUTO RANGE =====
    // минуты от 00:00 (0..1439)
    void setAutoRange(int startMin, int endMin);
    int  autoStart() const;
    int  autoEnd() const;

    // ===== RESULT =====
    bool isNight() const;

private:
    bool computeAutoNight(const TimeService& time) const;

private:
    UiVersionService&    _uiVersion;
    PreferencesService& _prefs;

    Mode _mode = Mode::AUTO;

    // дефолт (используется только если EEPROM пуст)
    int _autoStartMin = 22 * 60;
    int _autoEndMin   = 6 * 60;

    bool _isNight = false;
};