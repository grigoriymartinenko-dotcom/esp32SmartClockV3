#pragma once
#include "theme/Theme.h"

class Screen {
public:
    virtual ~Screen() = default;
    virtual void begin() = 0;
    virtual void update() = 0;

protected:
    const Theme& theme;
    explicit Screen(const Theme& t) : theme(t) {}
};
