#pragma once
#include <stdint.h>

/*
 * UiVersionService
 * ----------------
 * Единый источник версий UI-состояний.
 *
 * Принципы:
 *  - UI НИЧЕГО не считает
 *  - Сервисы bump'ают версии
 *  - Экран реагирует ТОЛЬКО на смену версии
 */

enum class UiChannel : uint8_t {
    TIME = 0,
    THEME,
    FORECAST,
    SENSOR,
    SCREEN,
    COUNT
};

class UiVersionService {
public:
    void begin();

    // bump версии канала
    void bump(UiChannel ch);

    // получить текущую версию канала
    uint32_t version(UiChannel ch) const;

private:
    uint32_t _versions[(uint8_t)UiChannel::COUNT] = {0};
};