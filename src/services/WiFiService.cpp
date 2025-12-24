#include "services/WifiService.h"

WifiService::WifiService(
    UiVersionService& ui,
    PreferencesService& prefs
)
    : _ui(ui)
    , _prefs(prefs)
{
}

void WifiService::begin() {
    _enabled = _prefs.wifiEnabled();
    if (_enabled) {
        start();
    } else {
        stop();
    }
}

void WifiService::update() {
    if (!_enabled)
        return;

    wl_status_t st = WiFi.status();

    if (st == WL_CONNECTED) {
        if (_state != State::ONLINE) {
            _state = State::ONLINE;
            _ui.bump(UiChannel::WIFI);
        }
        return;
    }

    if (_state == State::CONNECTING) {
        if (millis() - _connectStartMs > CONNECT_TIMEOUT_MS) {
            _state = State::ERROR;
            _ui.bump(UiChannel::WIFI);
        }
    }
}

void WifiService::setEnabled(bool on) {
    if (_enabled == on)
        return;

    _enabled = on;
    _prefs.setWifiEnabled(on);
    _prefs.save();

    on ? start() : stop();
}

bool WifiService::isEnabled() const {
    return _enabled;
}

WifiService::State WifiService::state() const {
    return _state;
}

void WifiService::start() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(); // SSID/PASS пока из памяти ESP32

    _connectStartMs = millis();
    _state = State::CONNECTING;
    _ui.bump(UiChannel::WIFI);
}

void WifiService::stop() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    _state = State::OFF;
    _ui.bump(UiChannel::WIFI);
}