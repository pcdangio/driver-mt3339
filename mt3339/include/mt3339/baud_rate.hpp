/// \file mt3339/baud_rate.hpp
/// \brief Defines the mt3339::baud_rate enumeration.
#ifndef MT3339___BAUD_RATE_H
#define MT3339___BAUD_RATE_H

namespace mt3339 {

/// \brief Enumerates the available baud rates for the MT3339.
enum class baud_rate
{
    B_4800 = 4800,
    B_9600 = 9600,
    B_14400 = 14400,
    B_19200 = 19200,
    B_38400 = 38400,
    B_57600 = 57600,
    B_115200 = 115200,
    B_230400 = 230400,
    B_460800 = 460800,
    B_921600 = 921600
};

}

#endif