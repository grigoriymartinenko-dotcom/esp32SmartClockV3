#pragma once

#include <time.h>
#include <stdint.h>

#include "services/UiVersionService.h"
#include "services/DstService.h"
#include "services/TimeProvider.h"

/*
 * TimeService
 * -----------
 * Единый источник времени для всей системы.
 *
 * UX helpers:
 * -----------
 * sourceLabel() / stateLabel()
 *   - ТОЛЬКО текст
 *   - БЕЗ цветов
 *   - БЕЗ логики UI
 */

class TimeService {
public:
    enum Mode {
        RTC_ONLY,
        NTP_ONLY,
        LOCAL_ONLY,
        AUTO
    };

    enum SyncState {
        NOT_STARTED,
        SYNCING,
        SYNCED,
        ERROR
    };

    enum Source {
        NONE,
        RTC,
        NTP
    };

    explicit TimeService(UiVersionService& uiVersion);

    void begin();
    void update();

    void registerProvider(TimeProvider& p);

    bool shouldWriteRtc() const;
    void markRtcWritten();

    void setMode(Mode m);
    Mode mode() const;

    void setTimezone(long gmtOffsetSec, int daylightOffsetSec);

    void setFromRtc(const tm& t);

    bool isValid() const;

    int hour()   const;
    int minute() const;
    int second() const;

    int day()   const;
    int month() const;
    int year()  const;

    SyncState syncState() const;
    Source    source()    const;

    // 👇 UX helpers (READ-ONLY)
    const char* sourceLabel() const;
    const char* stateLabel()  const;

    bool getTm(tm& out) const;

    bool isDstActive() const { return _dstActive; }

private:
    void updateFromSystemClock();
    void tryConsumeProviders();
    void applySystemTime(const tm& t);
    void syncNtp();
    void setSource(Source s);

    static bool looksValid(const tm& t);

private:
    UiVersionService& _uiVersion;

    Mode      _mode      = AUTO;
    SyncState _syncState = NOT_STARTED;
    Source    _source    = NONE;

    bool _ntpConfirmed = false;
    bool _valid        = false;
    bool _rtcWritten   = false;

    bool _rtcAppliedOnce = false;
    bool _ntpAppliedOnce = false;

    tm _timeinfo{};

    int _lastMinute = -1;
    int _lastSecond = -1;

    long _gmtOffsetSec      = 0;
    int  _daylightOffsetSec = 0;

    DstService _dst;
    bool _dstActive = false;

    unsigned long _syncStartedAt = 0;

    static constexpr uint8_t MAX_PROVIDERS = 4;
    TimeProvider* _providers[MAX_PROVIDERS]{};
    uint8_t _providersCount = 0;
};