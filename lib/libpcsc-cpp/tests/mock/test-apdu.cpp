// SPDX-FileCopyrightText: Estonian Information System Authority
// SPDX-License-Identifier: MIT

#include "pcsc-cpp/pcsc-cpp.hpp"

#include <gtest/gtest.h>

#include <algorithm>

using namespace pcsc_cpp;

#define CONSTEXPR_CHECK(...)                                                                       \
    static_assert((__VA_ARGS__));                                                                  \
    EXPECT_TRUE((__VA_ARGS__))

namespace
{

constexpr bool equals(const CommandApdu& apdu, std::initializer_list<byte_type> expected)
{
    const byte_vector& actual = apdu;
    return std::equal(actual.begin(), actual.end(), expected.begin(), expected.end());
}

constexpr bool equals(const ResponseApdu& response, byte_type sw1, byte_type sw2,
                      std::initializer_list<byte_type> data)
{
    return response.sw1 == sw1 && response.sw2 == sw2
        && std::equal(response.data.begin(), response.data.end(), data.begin(), data.end());
}
} // namespace

TEST(CommandApduTest, case1HeaderOnly)
{
    CONSTEXPR_CHECK(equals(CommandApdu {0x00, 0xA4, 0x04, 0x0C}, {0x00, 0xA4, 0x04, 0x0C}));
}

TEST(CommandApduTest, case2HeaderAndLe)
{
    CONSTEXPR_CHECK(
        equals(CommandApdu {0x00, 0xB0, 0x00, 0x00, 0x10}, {0x00, 0xB0, 0x00, 0x00, 0x10}));
}

TEST(CommandApduTest, case3HeaderLcAndData)
{
    CONSTEXPR_CHECK(equals(CommandApdu {0x00, 0xA4, 0x04, 0x0C, byte_vector {0xA0, 0x00, 0x01}},
                           {0x00, 0xA4, 0x04, 0x0C, 0x03, 0xA0, 0x00, 0x01}));
}

TEST(CommandApduTest, case4HeaderLcDataAndLe)
{
    CONSTEXPR_CHECK(equals(CommandApdu {0x00, 0x2A, 0x9E, 0x9A, byte_vector {0x01, 0x02}, 0x00},
                           {0x00, 0x2A, 0x9E, 0x9A, 0x02, 0x01, 0x02, 0x00}));
}

TEST(CommandApduTest, case3AcceptsMaxDataSize)
{
    CONSTEXPR_CHECK([] {
        CommandApdu apdu {0x00, 0xD6, 0x00, 0x00, byte_vector(CommandApdu::MAX_DATA_SIZE, 0xAA)};
        const byte_vector& d = apdu;
        return d.size() == CommandApdu::APDU_HEADER_AND_LC_SIZE + CommandApdu::MAX_DATA_SIZE
            && d[4] == 0xFF && d.back() == 0xAA;
    }());
}

TEST(CommandApduTest, case3RejectsDataOverMaxSize)
{
    EXPECT_THROW(
        (CommandApdu {0x00, 0xD6, 0x00, 0x00, byte_vector(CommandApdu::MAX_DATA_SIZE + 1, 0xAA)}),
        std::invalid_argument);
    EXPECT_THROW((CommandApdu {0x00, 0xD6, 0x00, 0x00,
                               byte_vector(CommandApdu::MAX_DATA_SIZE + 1, 0xAA), 0x00}),
                 std::invalid_argument);
}

TEST(CommandApduTest, case3RejectsEmptyData)
{
    EXPECT_THROW((CommandApdu {0x00, 0xA4, 0x04, 0x0C, byte_vector {}}), std::invalid_argument);
    EXPECT_THROW((CommandApdu {0x00, 0xA4, 0x04, 0x0C, byte_vector {}, 0x00}),
                 std::invalid_argument);
    EXPECT_THROW(CommandApdu::select(0x04, {}), std::invalid_argument);
    EXPECT_THROW(CommandApdu::selectEF(0x02, {}), std::invalid_argument);
}

TEST(CommandApduTest, withLeAppendsLeToCase1)
{
    CONSTEXPR_CHECK(equals(CommandApdu {CommandApdu {0x00, 0xCA, 0x01, 0x00}, 0x20},
                           {0x00, 0xCA, 0x01, 0x00, 0x20}));
}

TEST(CommandApduTest, withLeReplacesLeInCase2)
{
    CONSTEXPR_CHECK(equals(CommandApdu {CommandApdu {0x00, 0xB0, 0x00, 0x00, 0x00}, 0xE7},
                           {0x00, 0xB0, 0x00, 0x00, 0xE7}));
}

TEST(CommandApduTest, withLeAppendsLeToCase3)
{
    CONSTEXPR_CHECK(
        equals(CommandApdu {CommandApdu {0x00, 0x88, 0x00, 0x00, byte_vector {0x11}}, 0x00},
               {0x00, 0x88, 0x00, 0x00, 0x01, 0x11, 0x00}));
}

TEST(CommandApduTest, withLeReplacesLeInCase4)
{
    CONSTEXPR_CHECK(equals(
        CommandApdu {CommandApdu {0x00, 0x88, 0x00, 0x00, byte_vector {0x11, 0x22}, 0x00}, 0x40},
        {0x00, 0x88, 0x00, 0x00, 0x02, 0x11, 0x22, 0x40}));
}

TEST(CommandApduTest, select)
{
    CONSTEXPR_CHECK(equals(CommandApdu::select(0x08, {0x3F, 0x00, 0xAD, 0xF1}),
                           {0x00, 0xA4, 0x08, 0x0C, 0x04, 0x3F, 0x00, 0xAD, 0xF1}));
}

TEST(CommandApduTest, selectEF)
{
    CONSTEXPR_CHECK(equals(CommandApdu::selectEF(0x02, {0xD0, 0x01}),
                           {0x00, 0xA4, 0x02, 0x04, 0x02, 0xD0, 0x01, 0x00}));
}

TEST(CommandApduTest, readBinarySplitsOffsetIntoP1P2)
{
    CONSTEXPR_CHECK(equals(CommandApdu::readBinary(0x1234, 0xE7), {0x00, 0xB0, 0x12, 0x34, 0xE7}));
    CONSTEXPR_CHECK(equals(CommandApdu::readBinary(0, 0x00), {0x00, 0xB0, 0x00, 0x00, 0x00}));
}

TEST(CommandApduTest, getResponse)
{
    CONSTEXPR_CHECK(equals(CommandApdu::getResponse(), {0x00, 0xC0, 0x00, 0x00, 0x00}));
    CONSTEXPR_CHECK(equals(CommandApdu::getResponse(0x20), {0x00, 0xC0, 0x00, 0x00, 0x20}));
}

TEST(CommandApduTest, verifyPadsPin)
{
    CONSTEXPR_CHECK([] {
        byte_vector pin;
        pin.reserve(8 + CommandApdu::APDU_HEADER_AND_LC_SIZE);
        pin.insert(pin.end(), {'1', '2', '3', '4'});
        return equals(CommandApdu::verify(0x81, std::move(pin), 8, 0xFF),
                      {0x00, 0x20, 0x00, 0x81, 0x08, '1', '2', '3', '4', 0xFF, 0xFF, 0xFF, 0xFF});
    }());
}

TEST(CommandApduTest, verifyWithoutPaddingKeepsPin)
{
    CONSTEXPR_CHECK([] {
        byte_vector pin;
        pin.reserve(12 + CommandApdu::APDU_HEADER_AND_LC_SIZE);
        pin.insert(pin.end(), {'1', '2', '3', '4'});
        return equals(CommandApdu::verify(0x02, std::move(pin), 4, 0x00),
                      {0x00, 0x20, 0x00, 0x02, 0x04, '1', '2', '3', '4'});
    }());
}

TEST(CommandApduTest, verifyPaddingOnly)
{
    CONSTEXPR_CHECK(equals(CommandApdu::verify(0x85, 6, 0xFF),
                           {0x00, 0x20, 0x00, 0x85, 0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}));
}

TEST(CommandApduTest, verifyRejectsPinBufferWithoutPaddingCapacity)
{
    byte_vector pin {'1', '2', '3', '4'};
    EXPECT_THROW(CommandApdu::verify(0x81, std::move(pin), 64, 0xFF), std::invalid_argument);
}

TEST(CommandApduTest, verifyRejectsPinLongerThanPaddingLength)
{
    byte_vector pin;
    pin.reserve(16);
    pin.insert(pin.end(), {'1', '2', '3', '4', '5', '6', '7', '8', '9'});
    EXPECT_THROW(CommandApdu::verify(0x81, std::move(pin), 8, 0xFF), std::invalid_argument);
}

TEST(CommandApduTest, verifyRejectsEmptyPinWithoutPadding)
{
    EXPECT_THROW(CommandApdu::verify(0x81, 0, 0xFF), std::invalid_argument);
    EXPECT_THROW(CommandApdu::verify(0x81, byte_vector {}, 0, 0xFF), std::invalid_argument);
}

TEST(ResponseApduTest, fromBytesSplitsDataAndStatusBytes)
{
    CONSTEXPR_CHECK(
        equals(ResponseApdu::fromBytes({0xAB, 0xCD, 0x90, 0x00}), 0x90, 0x00, {0xAB, 0xCD}));
}

TEST(ResponseApduTest, fromBytesStatusBytesOnly)
{
    CONSTEXPR_CHECK(equals(ResponseApdu::fromBytes({0x6A, 0x82}), 0x6A, 0x82, {}));
    CONSTEXPR_CHECK(ResponseApdu::fromBytes({0x90, 0x00}).isOK());
}

TEST(ResponseApduTest, fromBytesAcceptsMaxSize)
{
    CONSTEXPR_CHECK([] {
        byte_vector bytes(ResponseApdu::MAX_SIZE, 0x11);
        bytes[ResponseApdu::MAX_DATA_SIZE] = 0x61;
        bytes[ResponseApdu::MAX_DATA_SIZE + 1] = 0x10;
        auto response = ResponseApdu::fromBytes(std::move(bytes));
        return response.sw1 == ResponseApdu::MORE_DATA_AVAILABLE && response.sw2 == 0x10
            && response.data.size() == ResponseApdu::MAX_DATA_SIZE && response.data.back() == 0x11;
    }());
}

TEST(ResponseApduTest, fromBytesRejectsInputShorterThanStatusBytes)
{
    EXPECT_THROW(ResponseApdu::fromBytes({}), std::invalid_argument);
    EXPECT_THROW(ResponseApdu::fromBytes({0x90}), std::invalid_argument);
}
