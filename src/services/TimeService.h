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
 * Ключевая идея (новая архитектура):
 *  - TimeService больше НЕ "ходит сам" за временем в разные места.
 *  - Он агрегирует TimeProvider'ы (RTC, NTP, любые будущие источники).
 *
 * Почему это "асинхронно":
 *  - NTP provider просто ждёт, когда системное время станет валидным (без блокировок).
 *  - RTC provider отдаёт время один раз сразу после старта.
 *
 * Режимы (оставляем твою API-совместимость):
 *  - RTC_ONLY   — использовать только RTC provider
 *  - NTP_ONLY   — использовать только NTP provider
 *  - LOCAL_ONLY — время не обновляется
 *  - AUTO       — RTC → затем уточнение NTP
 *
 * ПРАВИЛО:
 *  - _source — ЕДИНСТВЕННАЯ истина об активном источнике
 *  - Любая смена _source обязана дергать UiVersion::TIME
 *
 * ВАЖНО:
 *  - configTime(...) и DST-переключения остаются здесь (централизованно),
 *    чтобы в системе был единый "часовой пояс".
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

    // ===== Providers =====
    // Регистрируем провайдеры в нужном порядке приоритета.
    // Обычно:
    //   registerProvider(rtcProvider);
    //   registerProvider(ntpProvider);
    void registerProvider(TimeProvider& p);

    // ===== RTC sync policy =====
    bool shouldWriteRtc() const;
    void markRtcWritten();

    void setMode(Mode m);
    Mode mode() const;

    void setTimezone(long gmtOffsetSec, int daylightOffsetSec);

    // Оставляем для совместимости, но в новой архитектуре это "внешняя инъекция времени".
    // Обычно теперь RTC делает RtcTimeProvider.
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
    void updateFromSystemClock();      // тик + UI bump + DST
    void tryConsumeProviders();        // принять новое время от providers
    void applySystemTime(const tm& t); // установить системное время
    void syncNtp();                    // UX state: SYNCING
    void setSource(Source s);

private:
    UiVersionService& _uiVersion;

    Mode      _mode      = AUTO;
    SyncState _syncState = NOT_STARTED;
    Source    _source    = NONE;

    bool _ntpConfirmed = false;
    bool _valid        = false;
    bool _rtcWritten   = false;

    tm _timeinfo{};

    int _lastMinute = -1;
    int _lastSecond = -1;

    long _gmtOffsetSec      = 0;
    int  _daylightOffsetSec = 0;

    DstService _dst;
    bool _dstActive = false;

    // Providers (без dynamic allocation: фиксированный массив)
    static constexpr uint8_t MAX_PROVIDERS = 4;
    TimeProvider* _providers[MAX_PROVIDERS]{};
    uint8_t _providersCount = 0;
};