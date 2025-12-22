#include "services/DstService.h"

/*
 * Основная логика DST.
 * Работает строго по календарю, без таймеров.
 */
bool DstService::isDst(const tm& t) const {
    int year  = t.tm_year + 1900;
    int month = t.tm_mon + 1;
    int day   = t.tm_mday;
    int hour  = t.tm_hour;

    int marchLastSunday   = lastSunday(year, 3);
    int octoberLastSunday = lastSunday(year, 10);

    // Январь–Февраль → DST нет
    if (month < 3) return false;

    // Апрель–Сентябрь → DST всегда есть
    if (month > 3 && month < 10) return true;

    // Ноябрь–Декабрь → DST нет
    if (month > 10) return false;

    // ===== МАРТ =====
    // последнее воскресенье:
    //  - до 03:00 → ещё нет
    //  - с 03:00 → уже DST
    if (month == 3) {
        if (day > marchLastSunday) return true;
        if (day < marchLastSunday) return false;
        return hour >= 3;
    }

    // ===== ОКТЯБРЬ =====
    // последнее воскресенье:
    //  - до 04:00 → ещё DST
    //  - с 04:00 → DST выключен
    if (month == 10) {
        if (day < octoberLastSunday) return true;
        if (day > octoberLastSunday) return false;
        return hour < 4;
    }

    return false;
}

/*
 * Вычисление последнего воскресенья месяца.
 *
 * Трюк:
 *  - берём "день 0" следующего месяца
 *  - это последний день текущего
 *  - tm_wday = день недели (0 = воскресенье)
 */
int DstService::lastSunday(int year, int month) const {
    tm t{};
    t.tm_year = year - 1900;
    t.tm_mon  = month;   // следующий месяц
    t.tm_mday = 0;       // день 0 = последний день предыдущего
    mktime(&t);

    return t.tm_mday - t.tm_wday;
}