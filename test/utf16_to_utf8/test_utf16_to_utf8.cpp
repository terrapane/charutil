/*
 *  test_utf16_to_utf8.cpp
 *
 *  Copyright (c) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This module will test logic related converting strings from UTF-16LE to
 *      UTF-8.
 *
 *  Portability Issues:
 *      None.
 */

#include <cstdint>
#include <vector>
#include <algorithm>
#include <terra/charutil/character_utilities.h>
#include <terra/stf/adapters/integral_vector.h>
#include <terra/stf/stf.h>

using namespace Terra::CharUtil;

// It is assumed that a char and uint8_t are the same size
static_assert(sizeof(char) == sizeof(std::uint8_t));

STF_TEST(TestUTF16toUTF8, Empty)
{
    const std::vector<std::uint8_t> utf16le_string;
    std::vector<std::uint8_t> output;

    auto [result, length] = ConvertUTF16ToUTF8(
        std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t *>(utf16le_string.data()),
            utf16le_string.size()),
        output,
        true);

    // Ensure the conversion was successful
    STF_ASSERT_TRUE(result);

    // Verify the length
    STF_ASSERT_EQ(0, length);
}

STF_TEST(TestUTF16toUTF8, ASCII_1_LE)
{
    const std::u8string expected = u8"Hello";
    const std::vector<std::uint8_t> utf16le_string =
    {
        0x48, 0x00, 0x65, 0x00, 0x6c, 0x00, 0x6c, 0x00,
        0x6f, 0x00
    };

    std::vector<std::uint8_t> output(utf16le_string.size() * 1.5);
    auto [result, length] = ConvertUTF16ToUTF8(
        std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t *>(utf16le_string.data()),
            utf16le_string.size()),
        output,
        true);

    // Ensure the conversion was successful
    STF_ASSERT_TRUE(result);

    // Verify the length
    STF_ASSERT_EQ(expected.size(), length);

    // Verify the length <= vector size
    STF_ASSERT_LE(length, output.size());

    // Resize the vector to match the length
    output.resize(length);

    // Ensure the conversion is correct (copying "expected" into a vector)
    std::vector<std::uint8_t> expected_vec;
    std::copy(expected.begin(),
              expected.end(),
              std::back_inserter(expected_vec));
    STF_ASSERT_EQ(expected_vec, output);
}

STF_TEST(TestUTF16toUTF8, ASCII_1_BE)
{
    const std::u8string expected = u8"Hello";
    const std::vector<std::uint8_t> utf16be_string =
    {
         0x00, 0x48, 0x00, 0x65, 0x00, 0x6c, 0x00, 0x6c, 0x00, 0x6f
    };

    std::vector<std::uint8_t> output(utf16be_string.size() * 1.5);
    auto [result, length] = ConvertUTF16ToUTF8(
        std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t *>(utf16be_string.data()),
            utf16be_string.size()),
        output,
        false);

    // Ensure the conversion was successful
    STF_ASSERT_TRUE(result);

    // Verify the length
    STF_ASSERT_EQ(expected.size(), length);

    // Verify the length <= vector size
    STF_ASSERT_LE(length, output.size());

    // Resize the vector to match the length
    output.resize(length);

    // Ensure the conversion is correct (copying "expected" into a vector)
    std::vector<std::uint8_t> expected_vec;
    std::copy(expected.begin(),
              expected.end(),
              std::back_inserter(expected_vec));
    STF_ASSERT_EQ(expected_vec, output);
}

STF_TEST(TestUTF16toUTF8, ASCII_2)
{
    const std::u8string expected = u8"Hello, World!";
    const std::vector<std::uint8_t> utf16le_string =
    {
        0x48, 0x00, 0x65, 0x00, 0x6c, 0x00, 0x6c, 0x00,
        0x6f, 0x00, 0x2c, 0x00, 0x20, 0x00, 0x57, 0x00,
        0x6f, 0x00, 0x72, 0x00, 0x6c, 0x00, 0x64, 0x00,
        0x21, 0x00
    };

    std::vector<std::uint8_t> output(utf16le_string.size() * 1.5);
    auto [result, length] = ConvertUTF16ToUTF8(
        std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t *>(utf16le_string.data()),
            utf16le_string.size()),
        output,
        true);

    // Ensure the conversion was successful
    STF_ASSERT_TRUE(result);

    // Verify the length
    STF_ASSERT_EQ(expected.size(), length);

    // Verify the length <= vector size
    STF_ASSERT_LE(length, output.size());

    // Resize the vector to match the length
    output.resize(length);

    // Ensure the conversion is correct (copying "expected" into a vector)
    std::vector<std::uint8_t> expected_vec;
    std::copy(expected.begin(),
              expected.end(),
              std::back_inserter(expected_vec));
    STF_ASSERT_EQ(expected_vec, output);
}

STF_TEST(TestUTF16toUTF8, Chinese_LE)
{
    const std::u8string expected = u8"你好世界！";
    const std::vector<std::uint8_t> utf16le_string =
    {
        0x60, 0x4f, 0x7d, 0x59, 0x16, 0x4e, 0x4c, 0x75,
        0x01, 0xff
    };

    std::vector<std::uint8_t> output(utf16le_string.size() * 1.5);
    auto [result, length] = ConvertUTF16ToUTF8(
        std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t *>(utf16le_string.data()),
            utf16le_string.size()),
        output,
        true);

    // Ensure the conversion was successful
    STF_ASSERT_TRUE(result);

    // Verify the length
    STF_ASSERT_EQ(expected.size(), length);

    // Verify the length <= vector size
    STF_ASSERT_LE(length, output.size());

    // Resize the vector to match the length
    output.resize(length);

    // Ensure the conversion is correct (copying "expected" into a vector)
    std::vector<std::uint8_t> expected_vec;
    std::copy(expected.begin(),
              expected.end(),
              std::back_inserter(expected_vec));
    STF_ASSERT_EQ(expected_vec, output);
}

STF_TEST(TestUTF16toUTF8, Chinese_BE)
{
    const std::u8string expected = u8"你好世界！";
    const std::vector<std::uint8_t> utf16be_string =
    {
        0x4f, 0x60, 0x59, 0x7d, 0x4e, 0x16, 0x75, 0x4c, 0xff, 0x01
    };

    std::vector<std::uint8_t> output(utf16be_string.size() * 1.5);
    auto [result, length] = ConvertUTF16ToUTF8(
        std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t *>(utf16be_string.data()),
            utf16be_string.size()),
        output,
        false);

    // Ensure the conversion was successful
    STF_ASSERT_TRUE(result);

    // Verify the length
    STF_ASSERT_EQ(expected.size(), length);

    // Verify the length <= vector size
    STF_ASSERT_LE(length, output.size());

    // Resize the vector to match the length
    output.resize(length);

    // Ensure the conversion is correct (copying "expected" into a vector)
    std::vector<std::uint8_t> expected_vec;
    std::copy(expected.begin(),
              expected.end(),
              std::back_inserter(expected_vec));
    STF_ASSERT_EQ(expected_vec, output);
}

STF_TEST(TestUTF16toUTF8, Japanese)
{
    const std::u8string expected = u8"こんにちは世界！";
    const std::vector<std::uint8_t> utf16le_string =
    {
        0x53, 0x30, 0x93, 0x30, 0x6b, 0x30, 0x61, 0x30,
        0x6f, 0x30, 0x16, 0x4e, 0x4c, 0x75, 0x01, 0xff
    };

    std::vector<std::uint8_t> output(utf16le_string.size() * 1.5);
    auto [result, length] = ConvertUTF16ToUTF8(
        std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t *>(utf16le_string.data()),
            utf16le_string.size()),
        output,
        true);

    // Ensure the conversion was successful
    STF_ASSERT_TRUE(result);

    // Verify the length
    STF_ASSERT_EQ(expected.size(), length);

    // Verify the length <= vector size
    STF_ASSERT_LE(length, output.size());

    // Resize the vector to match the length
    output.resize(length);

    // Ensure the conversion is correct (copying "expected" into a vector)
    std::vector<std::uint8_t> expected_vec;
    std::copy(expected.begin(),
              expected.end(),
              std::back_inserter(expected_vec));
    STF_ASSERT_EQ(expected_vec, output);
}

STF_TEST(TestUTF16toUTF8, Korean)
{
    const std::u8string expected = u8"안녕하세요, 월드!";
    const std::vector<std::uint8_t> utf16le_string =
    {
        0x48, 0xc5, 0x55, 0xb1, 0x58, 0xd5, 0x38, 0xc1,
        0x94, 0xc6, 0x2c, 0x00, 0x20, 0x00, 0xd4, 0xc6,
        0xdc, 0xb4, 0x21, 0x00
    };

    std::vector<std::uint8_t> output(utf16le_string.size() * 1.5);
    auto [result, length] = ConvertUTF16ToUTF8(
        std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t *>(utf16le_string.data()),
            utf16le_string.size()),
        output,
        true);

    // Ensure the conversion was successful
    STF_ASSERT_TRUE(result);

    // Verify the length
    STF_ASSERT_EQ(expected.size(), length);

    // Verify the length <= vector size
    STF_ASSERT_LE(length, output.size());

    // Resize the vector to match the length
    output.resize(length);

    // Ensure the conversion is correct (copying "expected" into a vector)
    std::vector<std::uint8_t> expected_vec;
    std::copy(expected.begin(),
              expected.end(),
              std::back_inserter(expected_vec));
    STF_ASSERT_EQ(expected_vec, output);
}

STF_TEST(TestUTF16toUTF8, Russian)
{
    const std::u8string expected = u8"Привет, мир!";
    const std::vector<std::uint8_t> utf16le_string =
    {
        0x1f, 0x04, 0x40, 0x04, 0x38, 0x04, 0x32, 0x04,
        0x35, 0x04, 0x42, 0x04, 0x2c, 0x00, 0x20, 0x00,
        0x3c, 0x04, 0x38, 0x04, 0x40, 0x04, 0x21, 0x00
    };

    std::vector<std::uint8_t> output(utf16le_string.size() * 1.5);
    auto [result, length] = ConvertUTF16ToUTF8(
        std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t *>(utf16le_string.data()),
            utf16le_string.size()),
        output,
        true);

    // Ensure the conversion was successful
    STF_ASSERT_TRUE(result);

    // Verify the length
    STF_ASSERT_EQ(expected.size(), length);

    // Verify the length <= vector size
    STF_ASSERT_LE(length, output.size());

    // Resize the vector to match the length
    output.resize(length);

    // Ensure the conversion is correct (copying "expected" into a vector)
    std::vector<std::uint8_t> expected_vec;
    std::copy(expected.begin(),
              expected.end(),
              std::back_inserter(expected_vec));
    STF_ASSERT_EQ(expected_vec, output);
}

STF_TEST(TestUTF16toUTF8, Emoji_1)
{
    const std::u8string expected = u8"😀 Hello, World!😀 🌍";
    const std::vector<std::uint8_t> utf16le_string =
    {
        0x3d, 0xd8, 0x00, 0xde, 0x20, 0x00, 0x48, 0x00,
        0x65, 0x00, 0x6c, 0x00, 0x6c, 0x00, 0x6f, 0x00,
        0x2c, 0x00, 0x20, 0x00, 0x57, 0x00, 0x6f, 0x00,
        0x72, 0x00, 0x6c, 0x00, 0x64, 0x00, 0x21, 0x00,
        0x3d, 0xd8, 0x00, 0xde, 0x20, 0x00, 0x3c, 0xd8,
        0x0d, 0xdf
    };

    std::vector<std::uint8_t> output(utf16le_string.size() * 1.5);
    auto [result, length] = ConvertUTF16ToUTF8(
        std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t *>(utf16le_string.data()),
            utf16le_string.size()),
        output,
        true);

    // Ensure the conversion was successful
    STF_ASSERT_TRUE(result);

    // Verify the length
    STF_ASSERT_EQ(expected.size(), length);

    // Verify the length <= vector size
    STF_ASSERT_LE(length, output.size());

    // Resize the vector to match the length
    output.resize(length);

    // Ensure the conversion is correct (copying "expected" into a vector)
    std::vector<std::uint8_t> expected_vec;
    std::copy(expected.begin(),
              expected.end(),
              std::back_inserter(expected_vec));
    STF_ASSERT_EQ(expected_vec, output);
}

STF_TEST(TestUTF16toUTF8, Emoji_2)
{
    const std::u8string expected = u8"😀😁😂🤣😃😄😎🕵️‍♀️🧑‍💻🎈👖🍔☕🚌❤️🆗";
    const std::vector<std::uint8_t> utf16le_string =
    {
        0x3d, 0xd8, 0x00, 0xde, 0x3d, 0xd8, 0x01, 0xde,
        0x3d, 0xd8, 0x02, 0xde, 0x3e, 0xd8, 0x23, 0xdd,
        0x3d, 0xd8, 0x03, 0xde, 0x3d, 0xd8, 0x04, 0xde,
        0x3d, 0xd8, 0x0e, 0xde, 0x3d, 0xd8, 0x75, 0xdd,
        0x0f, 0xfe, 0x0d, 0x20, 0x40, 0x26, 0x0f, 0xfe,
        0x3e, 0xd8, 0xd1, 0xdd, 0x0d, 0x20, 0x3d, 0xd8,
        0xbb, 0xdc, 0x3c, 0xd8, 0x88, 0xdf, 0x3d, 0xd8,
        0x56, 0xdc, 0x3c, 0xd8, 0x54, 0xdf, 0x15, 0x26,
        0x3d, 0xd8, 0x8c, 0xde, 0x64, 0x27, 0x0f, 0xfe,
        0x3c, 0xd8, 0x97, 0xdd
    };

    std::vector<std::uint8_t> output(utf16le_string.size() * 1.5);
    auto [result, length] = ConvertUTF16ToUTF8(
        std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t *>(utf16le_string.data()),
            utf16le_string.size()),
        output,
        true);

    // Ensure the conversion was successful
    STF_ASSERT_TRUE(result);

    // Verify the length
    STF_ASSERT_EQ(expected.size(), length);

    // Verify the length <= vector size
    STF_ASSERT_LE(length, output.size());

    // Resize the vector to match the length
    output.resize(length);

    // Ensure the conversion is correct (copying "expected" into a vector)
    std::vector<std::uint8_t> expected_vec;
    std::copy(expected.begin(),
              expected.end(),
              std::back_inserter(expected_vec));
    STF_ASSERT_EQ(expected_vec, output);
}
