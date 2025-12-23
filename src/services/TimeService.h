#pragma once

#include <time.h>
#include <stdint.h>

#include "services/UiVersionService.h"
#include "services/DstService.h"

/*
 * TimeService
 * -----------
 * Единый источник времени для всей системы.
 *
 * Режимы:
 *  - RTC_ONLY   — использовать только RTC
 *  - NTP_ONLY   — использовать только NTP
 *  - LOCAL_ONLY — время не обновляется
 *  - AUTO       — RTC → затем уточнение NTP
 *
 * ПРАВИЛО:
 *  - _source — ЕДИНСТВЕННАЯ истина об активном источнике
 *  - Любая смена _source обязана дергать UiVersion::TIME
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

    // ===== RTC sync policy =====
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

    bool getTm(tm& out) const;

    bool isDstActive() const { return _dstActive; }

private:
    void updateTime();
    void syncNtp();

    bool _rtcWritten = false;

    // 🔹 ВАЖНО: централизованная установка источника
    void setSource(Source s);

private:
    UiVersionService& _uiVersion;

    Mode      _mode      = AUTO;
    SyncState _syncState = NOT_STARTED;
    Source    _source    = NONE;

    bool _ntpConfirmed = false;
    bool _valid        = false;

    tm _timeinfo{};

    int _lastMinute = -1;
    int _lastSecond = -1;

    long _gmtOffsetSec      = 0;
    int  _daylightOffsetSec = 0;

    DstService _dst;
    bool _dstActive = false;
};