/*
 *  test_utf8_length.cpp
 *
 *  Copyright (c) 2024
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      This module will test the function that gets the length of UTF-8
 *      strings.
 *
 *  Portability Issues:
 *      None.
 */

#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <cstddef>
#include <terra/charutil/character_utilities.h>
#include <terra/stf/adapters/integral_vector.h>
#include <terra/stf/stf.h>

using namespace Terra::CharUtil;

STF_TEST(TestUTF8Validity, ASCII)
{
    const std::u8string utf8_string = u8"Hello";

    STF_ASSERT_TRUE(IsUTF8Valid(std::span<const std::uint8_t>(
        reinterpret_cast<const std::uint8_t *>(utf8_string.data()),
        utf8_string.size())));
}

STF_TEST(TestUTF8Validity, Chinese)
{
    const std::u8string utf8_string = u8"你好世界！";

    STF_ASSERT_TRUE(IsUTF8Valid(std::span<const std::uint8_t>(
        reinterpret_cast<const std::uint8_t *>(utf8_string.data()),
        utf8_string.size())));
}

STF_TEST(TestUTF8Validity, Japanese)
{
    const std::u8string utf8_string = u8"こんにちは世界！";

    STF_ASSERT_TRUE(IsUTF8Valid(std::span<const std::uint8_t>(
        reinterpret_cast<const std::uint8_t *>(utf8_string.data()),
        utf8_string.size())));
}

STF_TEST(TestUTF8Validity, Korean)
{
    const std::u8string utf8_string = u8"안녕하세요, 월드!";

    STF_ASSERT_TRUE(IsUTF8Valid(std::span<const std::uint8_t>(
        reinterpret_cast<const std::uint8_t *>(utf8_string.data()),
        utf8_string.size())));
}

STF_TEST(TestUTF8Validity, Russian)
{
    const std::u8string utf8_string = u8"Привет, мир!";

    STF_ASSERT_TRUE(IsUTF8Valid(std::span<const std::uint8_t>(
        reinterpret_cast<const std::uint8_t *>(utf8_string.data()),
        utf8_string.size())));
}

STF_TEST(TestUTF8Validity, Emoji_1)
{
    const std::u8string utf8_string = u8"😀 Hello, World!😀 🌍";

    STF_ASSERT_TRUE(IsUTF8Valid(std::span<const std::uint8_t>(
        reinterpret_cast<const std::uint8_t *>(utf8_string.data()),
        utf8_string.size())));
}

STF_TEST(TestUTF8Validity, Emoji_2)
{
    const std::u8string utf8_string = u8"😀😁😂🤣😃😄😎🕵️‍♀️🧑‍💻🎈👖🍔"
                                        "☕🚌❤️🆗";

    STF_ASSERT_TRUE(IsUTF8Valid(std::span<const std::uint8_t>(
        reinterpret_cast<const std::uint8_t *>(utf8_string.data()),
        utf8_string.size())));
}

STF_TEST(TestUTF8Validity, Valid1)
{
    const std::vector<std::uint8_t> valid_sequence =
    {
        0xf0, 0x9f, 0x9a, 0xb5
    };

    STF_ASSERT_TRUE(IsUTF8Valid(valid_sequence));
}

STF_TEST(TestUTF8Validity, Valid2)
{
    const std::vector<std::uint8_t> valid_sequence =
    {
        // Person in boat
        0xf0, 0x9f, 0x9a, 0xa3,

        // Zero-Width Joiner
        0xe2, 0x80, 0x8d,

        // Female sign
        0xe2, 0x99, 0x80,

        // Variation selector 16
        0xef, 0xb8, 0x8f
    };

    STF_ASSERT_TRUE(IsUTF8Valid(valid_sequence));
}

STF_TEST(TestUTF8Validity, Valid3)
{
    STF_ASSERT_TRUE(IsUTF8Valid("Hello, World!"));
}

STF_TEST(TestUTF8Validity, Valid4)
{
    STF_ASSERT_TRUE(IsUTF8Valid(std::string("Hello, World!")));
}

STF_TEST(TestUTF8Validity, Valid5)
{
    STF_ASSERT_TRUE(IsUTF8Valid(u8"你好世界！"));
}

STF_TEST(TestUTF8Validity, Valid6)
{
    STF_ASSERT_TRUE(IsUTF8Valid(std::u8string(u8"你好世界！")));
}

STF_TEST(TestUTF8Validity, Valid7)
{
    std::array<char, 5> text = {'H', 'e', 'l', 'l', 'o'};

    STF_ASSERT_TRUE(IsUTF8Valid(text));
}

STF_TEST(TestUTF8Validity, Valid8)
{
    std::array<std::byte, 5> text = {std::byte{'H'},
                                     std::byte{'e'},
                                     std::byte{'l'},
                                     std::byte{'l'},
                                     std::byte{'o'}};

    STF_ASSERT_TRUE(IsUTF8Valid(text));
}

STF_TEST(TestUTF8Validity, Valid9)
{
    std::uint8_t text[] = {'H', 'e', 'l', 'l', 'o'};

    STF_ASSERT_TRUE(IsUTF8Valid(text));
}


STF_TEST(TestUTF8Validity, Invalid1)
{
    const std::vector<std::uint8_t> invalid_sequence =
    {
        // Person in boat (second octet wrong on purpose)
        0xf0, 0xdf, 0x9a, 0xa3,
    };

    STF_ASSERT_FALSE(IsUTF8Valid(invalid_sequence));
}

STF_TEST(TestUTF8Validity, Invalid2)
{
    const std::vector<std::uint8_t> invalid_sequence =
    {
        // Person in boat (last octet removed on purpose)
        0xf0, 0x9f, 0x9a
    };

    STF_ASSERT_FALSE(IsUTF8Valid(invalid_sequence));
}

STF_TEST(TestUTF8Validity, Invalid3)
{
    const std::vector<std::uint8_t> invalid_sequence =
    {
        // Person in boat (first octet wrong on purpose)
        0xf8, 0x9f, 0x9a, 0xa3
    };

    STF_ASSERT_FALSE(IsUTF8Valid(invalid_sequence));
}

STF_TEST(TestUTF8Validity, Invalid4)
{
    // Invalid in UTF-8 due to last octet
    std::byte invalid_sequence[] = {std::byte{'T'},
                                    std::byte{'e'},
                                    std::byte{'s'},
                                    std::byte{'t'},
                                    std::byte{0xFF}};

    STF_ASSERT_FALSE(IsUTF8Valid(invalid_sequence));
}

STF_TEST(TestUTF8Validity, Invalid5)
{
    // Invalid in UTF-8 due to last octet
    std::uint8_t invalid_sequence[] = {'T', 'e', 's', 't', 0xFF};

    STF_ASSERT_FALSE(IsUTF8Valid(invalid_sequence));
}
