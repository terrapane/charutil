/*
 *  utf8_to_utf16.cpp
 *
 *  Copyright (C) 2024, 2025
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      Utility function to convert from UTF-8 to UTF-16.
 *
 *  Portability Issues:
 *      None.
 */

#include <algorithm>
#include <iterator>
#include <terra/charutil/character_utilities.h>
#include "unicode_constants.h"

namespace Terra::CharUtil
{

namespace
{

/*
 *  InsertUTF16LE()
 *
 *  Description:
 *      This will take a UTF-16 charater and insert it into the specified buffer
 *      in little endian order.
 *
 *  Parameters:
 *      character [in]
 *          The UTF-16 character to insert into the buffer.
 *
 *      buffer [out]
 *          The buffer into which to write the character on little endian order.
 *
 *  Returns:
 *      Nothing, though the buffer will be populated with the value of the
 *      character in little endian order.
 *
 *  Comments:
 *      None.
 */
constexpr void InsertUTF16LE(const std::uint16_t character,
                             std::span<std::uint8_t, 2> buffer)
{
    buffer[0] = static_cast<uint8_t>(character & 0xff);
    buffer[1] = static_cast<uint8_t>((character >> 8) & 0xff);
}

/*
 *  InsertUTF16BE()
 *
 *  Description:
 *      This will take a UTF-16 charater and insert it into the specified buffer
 *      in big endian order.
 *
 *  Parameters:
 *      character [in]
 *          The UTF-16 character to insert into the buffer.
 *
 *      buffer [out]
 *          The buffer into which to write the character on big endian order.
 *
 *  Returns:
 *      Nothing, though the buffer will be populated with the value of the
 *      character in big endian order.
 *
 *  Comments:
 *      None.
 */
constexpr void InsertUTF16BE(const std::uint16_t character,
                             std::span<std::uint8_t, 2> buffer)
{
    buffer[0] = static_cast<uint8_t>((character >> 8) & 0xff);
    buffer[1] = static_cast<uint8_t>(character & 0xff);
}

} // namespace

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
                                            bool little_endian)
{
    std::size_t expected_utf8_remaining{};      // Number of UTF-8 octets left
    std::uint32_t wide_character{};             // UTF-32 character

    // If the input is zero length, so is the output
    if (in.empty()) return {true, 0};

    // If the output span is an insufficient size, return an error
    if (out.size() < (in.size() * 2)) return {false, 0};

    // Assign the output iterator
    std::span<uint8_t>::iterator p = out.begin();

    // Iterate over the UTF-8 string
    for (std::uint8_t octet : in)
    {
        // Handle subsequent UTF-8 octets
        if (expected_utf8_remaining > 0)
        {
            // Expecting a 10xxxxxx octet
            if ((octet & 0xc0) != 0x80) return {false, 0};

            // Append additional bits to the wide character
            wide_character = (wide_character << 6) | (octet & 0x3f);

            // Decrement the number of expected octets remaining
            expected_utf8_remaining--;

            // If this is the final UTF-8 character, produce the output
            if (expected_utf8_remaining == 0)
            {
                // Verify the character is <= 0x10'ffff per RFC 3629
                if (wide_character > Unicode::Maximum_Character_Value)
                {
                    return {false, 0};
                }

                // Ensure the character code is not within the surrogate range
                if ((wide_character >= Unicode::Surrogate_High_Min) &&
                    (wide_character <= Unicode::Surrogate_Low_Max))
                {
                    return {false, 0};
                }

                // Encode the Unicode character using surrogate code points
                // if it is between 0x10'0000 and 0x10'ffff.
                if (wide_character > Unicode::Maximum_BMP_Value)
                {
                    // Convert the code point values using two 16-bit values
                    // (See: https://www.Unicode.org/faq/utf_bom.html#utf16-3)
                    std::uint16_t high_surrogate =
                        static_cast<std::uint16_t>(Unicode::Lead_Offset +
                                                   (wide_character >> 10));
                    std::uint16_t low_surrogate =
                        static_cast<std::uint16_t>(Unicode::Surrogate_Low_Min +
                                                   (wide_character & 0x3ff));

                    if (little_endian)
                    {
                        InsertUTF16LE(high_surrogate,
                                      std::span<std::uint8_t, 2>{p, 2});
                        InsertUTF16LE(low_surrogate,
                                      std::span<std::uint8_t, 2>{p + 2, 2});
                    }
                    else
                    {
                        InsertUTF16BE(high_surrogate,
                                      std::span<std::uint8_t, 2>{p, 2});
                        InsertUTF16BE(low_surrogate,
                                      std::span<std::uint8_t, 2>{p + 2, 2});
                    }

                    // Advance the iterator p
                    p += 4;
                }
                else
                {
                    if (little_endian)
                    {
                        InsertUTF16LE(
                            static_cast<std::uint16_t>(wide_character),
                            std::span<std::uint8_t, 2>{p, 2});
                    }
                    else
                    {
                        InsertUTF16BE(
                            static_cast<std::uint16_t>(wide_character),
                            std::span<std::uint8_t, 2>{p, 2});
                    }

                    // Advance the iterator p
                    p += 2;
                }
            }

            continue;
        }

        // Single ASCII character?
        if (octet <= 0x7f)
        {
            if (little_endian)
            {
                InsertUTF16LE(octet, std::span<std::uint8_t, 2>{p, 2});
            }
            else
            {
                InsertUTF16BE(octet, std::span<std::uint8_t, 2>{p, 2});
            }
            p += 2;
            continue;
        }

        // Two octet UTF-8 sequence (110xxxxx)
        if ((octet & 0xe0) == 0xc0)
        {
            wide_character = octet & 0x3f;
            expected_utf8_remaining = 1;
            continue;
        }

        // Three octet UTF-8 sequence (1110xxxx)
        if ((octet & 0xf0) == 0xe0)
        {
            wide_character = octet & 0x0f;
            expected_utf8_remaining = 2;
            continue;
        }

        // Four octet UTF-8 sequence (11110xxx)
        if ((octet & 0xf8) == 0xf0)
        {
            wide_character = octet & 0x07;
            expected_utf8_remaining = 3;
            continue;
        }

        // Any other value would be an invalid UTF-8 value
        return {false, 0};
    }

    // If there are other octets expected, return an error
    if (expected_utf8_remaining > 0) return {false, 0};

    return {true, static_cast<std::size_t>(std::distance(out.begin(), p))};
}

} // namespace Terra::CharUtil
