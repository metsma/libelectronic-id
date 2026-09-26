// SPDX-FileCopyrightText: Estonian Information System Authority
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <sstream>
#include <iomanip>

namespace pcsc_cpp
{

/** Convert bytes to hex string. */
inline std::ostream& operator<<(std::ostream& os, const byte_vector& data)
{
    os << std::setfill('0') << std::hex;
    for (const auto byte : data)
        os << std::setw(2) << short(byte);
    return os << std::setfill(' ') << std::dec;
}

/** Convert the given integer to a hex string. */
template <typename T>
inline std::string int2hexstr(const T value)
{
    std::ostringstream hexStringBuilder;

    hexStringBuilder << "0x" << std::setfill('0') << std::setw(sizeof(long) * 2) << std::hex
                     << value;

    return hexStringBuilder.str();
}

} // namespace pcsc_cpp
