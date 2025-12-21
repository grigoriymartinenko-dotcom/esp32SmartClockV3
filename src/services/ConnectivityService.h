#pragma once
#include <WiFi.h>

#include "services/TimeService.h"
#include "ui/StatusBar.h"

/*
 * ConnectivityService
 * -------------------
 * ЕДИНЫЙ контроллер статусов подключения:
 *
 *  - Wi-Fi (CONNECTED / CONNECTING / ERROR)
 *  - NTP  (ONLINE / CONNECTING / ERROR / OFFLINE)
 *
 * Правила:
 *  - НЕ управляет UI напрямую, кроме StatusBar
 *  - НЕ знает про экраны
 *  - НЕ содержит delay() в update()
 *  - Вызывается из loop() один раз
 */

class ConnectivityService {
public:
    ConnectivityService(
        StatusBar& statusBar,
        TimeService& timeService
    );

    // инициализация (один раз в setup)
    void begin();

    // периодический апдейт (каждый loop)
    void update();

private:
    void updateWiFi();
    void updateNtp();

private:
    StatusBar&  _statusBar;
    TimeService& _time;

    // ===== Wi-Fi state tracking =====
    wl_status_t _lastWiFiStatus = WL_IDLE_STATUS;

    // ===== NTP state tracking =====
    TimeService::SyncState _lastNtpState = TimeService::NOT_STARTED;
    bool _ntpEverSynced = false;
};