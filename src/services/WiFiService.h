#pragma once

#include <WiFi.h>

/*
 * ============================================================
 * WiFiService
 *
 * Сервис управления Wi-Fi:
 *  - подключение
 *  - переподключение
 *  - предоставление статуса
 *
 * НЕ знает про UI, экраны, погоду.
 * ============================================================
 */
class WiFiService {
public:
    enum class Status {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    };

    WiFiService(const char* ssid, const char* password);

    // запуск подключения
    void begin();

    // вызывать в loop()
    void update();

    // состояния
    bool isConnected() const;
    Status status() const;

private:
    const char* _ssid;
    const char* _password;

    Status _status = Status::DISCONNECTED;

    unsigned long _lastAttemptMs = 0;
    static constexpr unsigned long RECONNECT_INTERVAL = 10000; // 10 сек
};