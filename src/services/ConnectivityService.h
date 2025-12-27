#pragma once
#include <WiFi.h>

#include "services/TimeService.h"

/*
 * ConnectivityService
 * -------------------
 * Контроллер сетевых состояний:
 *
 *  - Wi-Fi (через WiFi.status())
 *  - NTP  (через TimeService)
 *
 * Правила:
 *  - НЕ управляет UI
 *  - НЕ знает про StatusBar
 *  - НЕ знает про экраны
 */

class ConnectivityService {
public:
    explicit ConnectivityService(
        TimeService& timeService
    );

    void begin();
    void update();

private:
    void updateWiFi();
    void updateNtp();

private:
    TimeService& _time;

    wl_status_t _lastWiFiStatus = WL_IDLE_STATUS;
    TimeService::SyncState _lastNtpState = TimeService::NOT_STARTED;
    bool _ntpEverSynced = false;
};