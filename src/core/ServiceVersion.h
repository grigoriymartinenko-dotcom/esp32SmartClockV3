#pragma once
#include <stdint.h>

/*
 * ServiceVersion
 * --------------
 * Монотонный счётчик изменений сервиса.
 * bump() вызывается ТОЛЬКО если состояние реально изменилось.
 */
struct ServiceVersion {
    uint32_t value = 0;
    void bump() { ++value; }
};