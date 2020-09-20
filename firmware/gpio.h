#pragma once

#include "hal.h"

class Gpio
{
public:
    Gpio(ioline_t line) : m_line(line) {};

    void Set()
    {
        palSetLine(m_line);
    }

    void Clear()
    {
        palClearLine(m_line);
    }

private:
    const ioline_t m_line;
};
