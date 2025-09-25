/*
 *  character_utilities.h
 *
 *  Copyright (C) 2024, 2025
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      Utility functions to assist in converting between UTF-8 and UTF-16
 *      (either little endian or big endian).
 *
 *  Portability Issues:
 *      None.
 */

#pragma once

#include <string>
#include <span>
#include <utility>
#include <cstdint>
#include <cstddef>
#include <climits>
#include <limits>
#include <concepts>
#include <ranges>

namespace Terra::CharUtil
{

// Define a concept to represent any 8-bit integral value
template<typename T>
concept EightBitIntegral = std::integral<T> && sizeof(T) == 1;

// Define concept to facilitate using template functions with strings
template<typename R>
concept ContiguousEightBitRange =
    std::ranges::contiguous_range<R> &&
    EightBitIntegral<std::ranges::range_value_t<R>>;

// Define the UTF-8/16 Byte Order Mark (BOM) value; this would be encoded in
// files or data transmission as 0xFFEE for Big Endian and 0xFEFF and as
// 0xFFFE for Little Endian and appears as the sequence 0xEF BB BF in UTF-8
constexpr std::uint16_t UTF_BOM = 0xFEFF;

// Define the maximum length of a UTF-16 string when converting to UTF-8
// supported by the ConvertUTF16ToUTF8() function
//
// The definition that follows is roughly this:
//
// constexpr std::size_t Max_UTF16_String =
//                                std::numeric_limits<std::size_t>::max() / 1.5;
//
// This definition is more precise for (2^n - 1) where n is even, as the math
// produces a bit pattern of 10101010...10.  What this definition does is solve
// that math problem of n * 2 / 3 twice by performing the math in the lower
// n/2 bits, then shifting one of the results up into the upper n/2 bits to
// produce a complete value.
constexpr std::size_t Max_UTF16_String =
    ((((std::numeric_limits<std::size_t>::max() >>
        ((sizeof(std::size_t) * CHAR_BIT) >> 1)) << 1) / 3) <<
        ((sizeof(std::size_t) * CHAR_BIT) >> 1)) |
    (((std::numeric_limits<std::size_t>::max() >>
        ((sizeof(std::size_t) * CHAR_BIT) >> 1)) << 1) / 3);

/*
 *  ConvertUTF8ToUTF16()
 *
 *  Description:
 *      This function will take a span of octets in UTF-8 format and convert
 *      them to UTF-16 format.  This function will not insert byte-order-mark
 *      (BOM) octets, though if one exists in the input UTF-8 string it will
 *      be retained.  The desired endianness for the UTF-16 output is specified
 *      via the third parameter.
 *
 *  Parameters:
 *      in [in]
 *          Original string in UTF-8 format.
 *
 *      out [out]
 *          The UTF-16 string derived from the given UTF-8 string.  This span
 *          MUST be at least 2x larger than the input span, though the encoding
 *          length might be smaller.
 *
 *      little_endian [in]
 *          Store the UTF-16 characters in little endian order?  This defaults
 *          to true, as nearly every modern platform uses Little Endian.
 *
 *  Returns:
 *      A boolean and length pair, where the boolean indicates success or
 *      failure to convert the UTF-8 string.  Only if the return result is true
 *      does the length value have meaning.  On success, the length value
 *      indicates the number of octets (not characters!) in the resulting
 *      UTF-16 output span.
 *
 *  Comments:
 *      None.
 */
std::pair<bool, std::size_t> ConvertUTF8ToUTF16(
                                            std::span<const std::uint8_t> in,
                                            std::span<std::uint8_t> out,
                                            bool little_endian = true);

// Same as above, but allowing any range holding 8-bit values
template<ContiguousEightBitRange R1, ContiguousEightBitRange R2>
inline std::pair<bool, std::size_t> ConvertUTF8ToUTF16(
                                            R1 &&range1,
                                            R2 &&range2,
                                            bool little_endian = true)
{
    return ConvertUTF8ToUTF16(
        std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t *>(std::ranges::data(range1)),
            std::ranges::size(range1)),
        std::span<std::uint8_t>(
            reinterpret_cast<std::uint8_t *>(std::ranges::data(range2)),
            std::ranges::size(range2)),
        little_endian);
}

/*
 *  ConvertUTF16ToUTF8()
 *
 *  Description:
 *      This function will take a span of octets in UTF-16 format and convert
 *      them to UTF-8 format.  The UTF-16 octets may have a byte-order-mark
 *      (BOM) at the start, in which case the "little_endian" parameter will
 *      be ignored and endianness derived from the BOM value.  The endianness
 *      is indicated via the third argument if no BOM exists in the input data.
 *      If a BOM is present in the input string, it will be preserved in the
 *      output stream.  For UTF-8, the BOM is encoded as 0xEF BB BF.
 *
 *  Parameters:
 *      in [in]
 *          The user-provided UTF-16 string.  This parameters must be less
 *          than (2^n - 1) / 1.5, where n is the bit-length of the type
 *          std::size_t. The reason is that the output buffer must be 1.5x
 *          larger than the input buffer.  The length will be checked to
 *          ensure the length is not greater thant Max_UTF16_String.
 *
 *      out [out]
 *          The UTF-8 string derived from the given UTF-16 string.  This span
 *          MUST be 50% larger than the length of the input string since
 *          characters in the range of 0x0800 to 0xFFFF consumes two octets as
 *          UTF-16 and three octets when encoded as UTF-8.  While UTF-8 can
 *          encode supplementary beyond the Basic Multilingual Plane (BMP)
 *          require four octets to encode, those are represented as surrogate
 *          pairs in UTF-16 that are already four octets in length, thus
 *          they do not expand the length of the output string.
 *
 *      little_endian [in]
 *          Are the UTF-16 characters in little endian order? This defaults to
 *          true, as nearly every modern platform uses Little Endian.  However,
 *          if the input data has a BOM at the front, the BOM value will
 *          takes precedence over this argument.
 *
 *  Returns:
 *      A boolean and length pair, where the boolean indicates success or
 *      failure to convert the UTF-16 string.  Only if the return result is
 *      true does the length value have meaning.  On success, the length value
 *      indicates the number of octets (not characters!) in the resulting
 *      UTF-8 output span.
 *
 *  Comments:
 *      None.
 */
std::pair<bool, std::size_t> ConvertUTF16ToUTF8(
                                            std::span<const std::uint8_t> in,
                                            std::span<std::uint8_t> out,
                                            bool little_endian = true);

// Same as above, but allowing any range holding 8-bit values
template<ContiguousEightBitRange R1, ContiguousEightBitRange R2>
inline std::pair<bool, std::size_t> ConvertUTF16ToUTF8(
                                            R1 &&range1,
                                            R2 &&range2,
                                            bool little_endian = true)
{
    return ConvertUTF16ToUTF8(
        std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t *>(std::ranges::data(range1)),
            std::ranges::size(range1)),
        std::span<std::uint8_t>(
            reinterpret_cast<std::uint8_t *>(std::ranges::data(range2)),
            std::ranges::size(range2)),
        little_endian);
}

/*
 *  IsUTF8Valid()
 *
 *  Description:
 *      This function will process the sequence of octets and verify that it
 *      is a valid UTF-8 sequence.  By "valid", it means the sequence of
 *      octets conform to the encoding described in IETF RFC 3629.
 *
 *  Parameters:
 *      octets [in]
 *          The sequence of octets to process.
 *
 *  Returns:
 *      True if the octet sequence is a valid UTF-8 sequence or false otherwise.
 *
 *  Comments:
 *      This  does not ensure that the arrangement of a sequence of Unicode code
 *      points have a valid meaning or would result in the rendering of a
 *      character.  For example, while a Zero Width Joiner (ZWJ) can be used to
 *      join two or more code points to produce a single, visible character on
 *      the screen, this function makes no attempt to verify that such sequences
 *      make sense.
 */
bool IsUTF8Valid(std::span<const std::uint8_t> octets);

// Same as above, but allowing any range holding 8-bit values
template<ContiguousEightBitRange R>
inline bool IsUTF8Valid(R &&range)
{
    return IsUTF8Valid(std::span<const std::uint8_t>(
        reinterpret_cast<const std::uint8_t *>(std::ranges::data(range)),
        std::ranges::size(range)));
}

} // namespace Terra::CharUtil
