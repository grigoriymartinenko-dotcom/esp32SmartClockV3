#include "services/ConnectivityService.h"

ConnectivityService::ConnectivityService(
    StatusBar& statusBar,
    TimeService& timeService
)
    : _statusBar(statusBar)
    , _time(timeService)
{
}

void ConnectivityService::begin() {
    // начальное состояние Wi-Fi
    _statusBar.setWiFiStatus(StatusBar::CONNECTING);

    // начальное состояние NTP
    _statusBar.setNtpStatus(StatusBar::OFFLINE);

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

    switch (st) {
        case WL_CONNECTED:
            _statusBar.setWiFiStatus(StatusBar::ONLINE);
            break;

        case WL_IDLE_STATUS:
        case WL_DISCONNECTED:
        case WL_CONNECTION_LOST:
            _statusBar.setWiFiStatus(StatusBar::CONNECTING);
            break;

        case WL_NO_SSID_AVAIL:
        case WL_CONNECT_FAILED:
        default:
            _statusBar.setWiFiStatus(StatusBar::ERROR);
            break;
    }
}

// =====================================================
// NTP
// =====================================================
void ConnectivityService::updateNtp() {

    TimeService::SyncState st = _time.syncState();
    if (st == _lastNtpState)
        return;

    _lastNtpState = st;

    switch (st) {
        case TimeService::SYNCED:
            _ntpEverSynced = true;
            _statusBar.setNtpStatus(StatusBar::ONLINE);
            break;

        case TimeService::SYNCING:
            _statusBar.setNtpStatus(StatusBar::CONNECTING);
            break;

        case TimeService::ERROR:
            _statusBar.setNtpStatus(StatusBar::ERROR);
            break;

        case TimeService::NOT_STARTED:
        default:
            _statusBar.setNtpStatus(
                _ntpEverSynced ? StatusBar::ONLINE
                               : StatusBar::OFFLINE
            );
            break;
    }
}