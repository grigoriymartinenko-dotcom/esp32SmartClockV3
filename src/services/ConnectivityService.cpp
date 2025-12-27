#include "services/ConnectivityService.h"

ConnectivityService::ConnectivityService(
    TimeService& timeService
)
    : _time(timeService)
{
}

void ConnectivityService::begin() {
    _lastWiFiStatus = WiFi.status();
    _lastNtpState   = TimeService::NOT_STARTED;
    _ntpEverSynced  = false;
}

void ConnectivityService::update() {
    updateWiFi();
    updateNtp();
}

// =====================================================
// Wi-Fi
// =====================================================
void ConnectivityService::updateWiFi() {

    wl_status_t st = WiFi.status();
    if (st == _lastWiFiStatus)
        return;

    _lastWiFiStatus = st;
}

// =====================================================
// NTP
// =====================================================
void ConnectivityService::updateNtp() {

    TimeService::SyncState st = _time.syncState();
    if (st == _lastNtpState)
        return;

    _lastNtpState = st;

    if (st == TimeService::SYNCED)
        _ntpEverSynced = true;
}