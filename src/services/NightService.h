#pragma once
#include <stdint.h>

#include "services/UiVersionService.h"
#include "services/TimeService.h"

/*
 * NightService
 * ------------
 * Night Mode:
 *  - AUTO: ночь по пользовательскому интервалу времени
 *  - ON:   всегда ночь
 *  - OFF:  всегда день
 *
 * Истина ОДНА: NightService решает, ночь сейчас или нет.
 * Любая смена состояния → UiChannel::THEME
 */

class NightService {
public:
    enum class Mode : uint8_t {
        AUTO = 0,
        ON,
        OFF
    };

    explicit NightService(UiVersionService& uiVersion);

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
    UiVersionService& _uiVersion;

    Mode _mode = Mode::AUTO;

    // дефолт: 22:00 → 06:00
    int _autoStartMin = 22 * 60;
    int _autoEndMin   = 6 * 60;

    bool _isNight = false;
};