/*
 * Copyright (c) 2020-2024 Estonian Information System Authority
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "pcsc-cpp/pcsc-cpp.hpp"
#include "pcsc-cpp/pcsc-cpp-utils.hpp"

#include <sstream>
#include <iomanip>

using namespace pcsc_cpp;
using namespace std::string_literals;

#ifdef HIBYTE
#undef HIBYTE
#endif
#ifdef LOBYTE
#undef LOBYTE
#endif

constexpr byte_type HIBYTE(size_t w) noexcept
{
    return static_cast<byte_type>((w >> 8) & 0xff);
}
constexpr byte_type LOBYTE(size_t w) noexcept
{
    return static_cast<byte_type>(w & 0xff);
}

namespace
{

const byte_type DER_SEQUENCE_TYPE_TAG = 0x30;
const byte_type DER_TWO_BYTE_LENGTH = 0x82;

class UnexpectedResponseError : public Error
{
public:
    explicit UnexpectedResponseError(const CommandApdu& command, const ResponseApdu& response,
                                     const char* file, const int line,
                                     const char* callerFunctionName) :
        Error("transmitApduWithExpectedResponse(): Unexpected response to command '"s + command
              + "' - expected '9000', got '"s + response + "' in " + removeAbsolutePathPrefix(file)
              + ':' + std::to_string(line) + ':' + callerFunctionName)
    {
    }
};

} // namespace

namespace pcsc_cpp
{

std::ostream& operator<<(std::ostream& os, const pcsc_cpp::byte_vector& data)
{
    os << std::setfill('0') << std::hex;
    for (const auto byte : data)
        os << std::setw(2) << short(byte);
    return os << std::setfill(' ') << std::dec;
}

std::string operator+(std::string lhs, const byte_vector& rhs)
{
    lhs.reserve(lhs.size() + rhs.size() * 2);
    std::ostringstream hexStringBuilder(std::move(lhs), std::ios::ate);
    hexStringBuilder << rhs;
    return hexStringBuilder.str();
}

void transmitApduWithExpectedResponse(const SmartCard& card, const CommandApdu& command)
{
    const auto response = card.transmit(command);
    if (!response.isOK()) {
        throw UnexpectedResponseError(command, response, __FILE__, __LINE__, __func__);
    }
}

size_t readDataLengthFromAsn1(const SmartCard& card)
{
    // p1 - offset size first byte, 0
    // p2 - offset size second byte, 0
    // le - number of bytes to read, need 4 bytes from start for length
    const CommandApdu readBinary4Bytes {0x00, 0xb0, 0x00, 0x00, 0x04};

    auto response = card.transmit(readBinary4Bytes);

    // Verify expected DER header, first byte must be SEQUENCE.
    if (response.data[0] != DER_SEQUENCE_TYPE_TAG) {
        // TODO: more specific exception
        THROW(Error,
              "readDataLengthFromAsn1(): First byte must be SEQUENCE (0x30), but is "s
                  + int2hexstr(response.data[0]));
    }

    // TODO: support other lenghts besides 2.
    // Assume 2-byte length, so second byte must be 0x82.
    if (response.data[1] != DER_TWO_BYTE_LENGTH) {
        // TODO: more specific exception
        THROW(Error,
              "readDataLengthFromAsn1(): Second byte must be two-byte length indicator "s
              "(0x82), but is "s
                  + int2hexstr(response.data[1]));
    }

    // Read 2-byte length field at offset 2 and 3 and add the 4 DER length bytes.
    const auto length = size_t((response.data[2] << 8) + response.data[3] + 4);
    if (length < 128 || length > 0x0f00) {
        // TODO: more specific exception
        THROW(Error,
              "readDataLengthFromAsn1(): Unexpected data length in DER header: "s
                  + std::to_string(length));
    }

    return length;
}

byte_vector readBinary(const SmartCard& card, const size_t length, byte_type blockLength)
{
    auto lengthCounter = length;
    auto resultBytes = byte_vector {};

    for (size_t offset = 0; lengthCounter != 0;
         offset += blockLength, lengthCounter -= blockLength) {

        if (blockLength > lengthCounter) {
            blockLength = byte_type(lengthCounter);
        }

        CommandApdu readBinary {0x00, 0xb0, HIBYTE(offset), LOBYTE(offset), blockLength};
        auto response = card.transmit(readBinary);

        resultBytes.insert(resultBytes.end(), response.data.cbegin(), response.data.cend());
    }

    if (resultBytes.size() != length) {
        // TODO: more specific exception
        THROW(Error, "readBinary(): Invalid length: "s + std::to_string(resultBytes.size()));
    }

    return resultBytes;
}

} // namespace pcsc_cpp
