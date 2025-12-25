#pragma once
#include <stdint.h>

/*
 * UiVersionService
 * ----------------
 * Сервис "версий" для реактивной перерисовки UI.
 *
 * Идея:
 *  - любой сервис (WiFi/Time/Theme/Screen...) делает bump(channel)
 *  - экран/виджет в update() проверяет changed(channel)
 *  - если changed == true → ставим _dirty и перерисовываем
 *
 * ВАЖНО:
 *  - changed(channel) "потребляет" изменение (consume)
 *    т.е. второй вызов подряд вернёт false, пока снова не будет bump()
 */

enum class UiChannel : uint8_t {
    WIFI = 0,
    TIME,
    THEME,
    SCREEN,
    DHT,        // 👈 ДОБАВИТЬ
    COUNT
};

class UiVersionService {
public:
    UiVersionService();
    void begin();
    // Увеличить версию канала (сигнал "что-то поменялось")
    void bump(UiChannel ch);

    // Текущая версия канала (иногда полезно для отладки)
    uint32_t version(UiChannel ch) const;

    /*
     * changed(ch)
     * ----------
     * Возвращает true если версия канала изменилась
     * с момента последней проверки changed(ch).
     *
     * Это именно то, чего не хватало SettingsScreen.
     */
    bool changed(UiChannel ch);

private:
    static constexpr uint8_t N = (uint8_t)UiChannel::COUNT;

    uint32_t _v[N];       // текущие версии
    uint32_t _seen[N];    // последняя "увиденная" версия (для changed)
};