#pragma once
#include <time.h>

/*
 * DstService
 * ----------
 * AUTO DST по правилам ЕС (актуально для Украины):
 *
 *  - ВКЛ (DST = ON):
 *      последнее воскресенье марта, с 03:00
 *
 *  - ВЫКЛ (DST = OFF):
 *      последнее воскресенье октября, с 04:00
 *
 * ВАЖНО:
 *  - сервис СТАТЛЕСС
 *  - не хранит время
 *  - не использует RTC / NTP / millis
 *  - работает ТОЛЬКО по переданному tm
 *
 * TimeService решает, КОГДА его вызывать.
 */
class DstService {
public:
    // Возвращает true, если DST должен быть активен
    bool isDst(const tm& t) const;

private:
    // Вычисляет день месяца последнего воскресенья
    // month: 1..12
    int lastSunday(int year, int month) const;
};