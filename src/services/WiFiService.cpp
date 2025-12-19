#include "services/WiFiService.h"

/*
 * ============================================================
 * Реализация WiFiService
 * ============================================================
 */

WiFiService::WiFiService(const char* ssid, const char* password)
: _ssid(ssid),
  _password(password)
{}

void WiFiService::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);
    _status = Status::CONNECTING;
    _lastAttemptMs = millis();
}

void WiFiService::update() {
    wl_status_t s = WiFi.status();

    if (s == WL_CONNECTED) {
        _status = Status::CONNECTED;
        return;
    }

    unsigned long now = millis();

    // если отвалились — пробуем переподключиться раз в RECONNECT_INTERVAL
    if (now - _lastAttemptMs >= RECONNECT_INTERVAL) {
        _lastAttemptMs = now;
        WiFi.disconnect();
        WiFi.begin(_ssid, _password);
        _status = Status::CONNECTING;
    }
}

bool WiFiService::isConnected() const {
    return _status == Status::CONNECTED;
}

WiFiService::Status WiFiService::status() const {
    return _status;
}