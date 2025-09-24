/*
 *  utf16_to_utf8.cpp
 *
 *  Copyright (C) 2024, 2025
 *  Terrapane Corporation
 *  All Rights Reserved
 *
 *  Author:
 *      Paul E. Jones <paulej@packetizer.com>
 *
 *  Description:
 *      Utility function to convert from UTF-16 to UTF-8.
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
 *  ExtractUTF16LE()
 *
 *  Description:
 *      This will take extract a UTF-16 charater (or surrogate) from the
 *      specified buffer stored in little endian order.
 *
 *  Parameters:
 *      buffer [in]
 *          The buffer into which to read the character.
 *
 *  Returns:
 *      The UTF-16 character extracted from the buffer.  The character is stored
 *      is host order.
 *
 *  Comments:
 *      None.
 */
constexpr std::uint16_t ExtractUTF16LE(std::span<const std::uint8_t, 2> buffer)
{
    return (static_cast<std::uint16_t>(buffer[1]) << 8) |
           (static_cast<std::uint16_t>(buffer[0])     );
}

/*
 *  ExtractUTF16BE()
 *
 *  Description:
 *      This will take extract a UTF-16 charater (or surrogate) from the
 *      specified buffer stored in big endian order.
 *
 *  Parameters:
 *      buffer [in]
 *          The buffer into which to read the character.
 *
 *  Returns:
 *      The UTF-16 character extracted from the buffer.  The character is stored
 *      is host order.
 *
 *  Comments:
 *      None.
 */
constexpr std::uint16_t ExtractUTF16BE(std::span<const std::uint8_t, 2> buffer)
{
    return (static_cast<std::uint16_t>(buffer[0]) << 8) |
           (static_cast<std::uint16_t>(buffer[1])     );
}

} // namespace

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
                                            bool little_endian)
{
    // If the input is zero length, so is the output
    if (in.size() == 0) return {true, 0};

    // UTF-16 always has an even number of octets, so verify that is the case
    if ((in.size() & 1) != 0) return {false, 0};

    // Reject UTF-16 strings that are to long
    if (in.size() > Max_UTF16_String) return {false, 0};

    // If the output span is an insufficient size, return an error (1.5x size)
    if (out.size() < (in.size() + (in.size() >> 1))) return {false, 0};

    // Assign the input and output iterators
    std::span<const uint8_t>::iterator p = in.begin();
    std::span<const uint8_t>::iterator q = in.end();
    std::span<uint8_t>::iterator r = out.begin();

    // Check to see if there is a BOM at the start of the string and, if so,
    // change the endian flag as appropriate
    if (std::distance(p, q) >= 4)
    {
        // Check if this is in Big Endian order, the little endian
        if ((*(p + 0) == 0xFE) && (*(p + 1) == 0xFF))
        {
            little_endian = false;
        }
        else if ((*(p + 0) == 0xFF) && (*(p + 1) == 0xFE))
        {
            little_endian = true;
        }
    }

    // Iterate over the input span
    while (p != q)
    {
        std::uint32_t character{};

        // Extract the character from the input span (uint32_t is used since
        // since UTF-16 can encode characters in the range of 0..1ffff using)
        // surrogate code point values
        if (little_endian)
        {
            character = ExtractUTF16LE(std::span<const uint8_t, 2>(p, 2));
        }
        else
        {
            character = ExtractUTF16BE(std::span<const uint8_t, 2>(p, 2));
        }

        // Advance the iterator to the next character (or surrogate)
        p += 2;

        // Is this character code in the surrogate range?
        if ((character >= Unicode::Surrogate_High_Min) &&
            (character <= Unicode::Surrogate_Low_Max))
        {
            std::uint16_t low_surrogate{};

            // Ensure the character value is not in the low surrogate range
            if ((character >= Unicode::Surrogate_Low_Min) &&
                (character <= Unicode::Surrogate_Low_Max))
            {
                return {false, 0};
            }

            // Ensure we do not run off the end of the buffer as we read the
            // low surrogate value
            if (p == q) return {false, 0};

            // Extract the low surrogate code point
            if (little_endian)
            {
                low_surrogate =
                    ExtractUTF16LE(std::span<const uint8_t, 2>(p, 2));
            }
            else
            {
                low_surrogate =
                    ExtractUTF16BE(std::span<const uint8_t, 2>(p, 2));
            }

            // Advance the iterator to the next character
            p += 2;

            // Ensure the low surrogate value is within the expected range
            if ((low_surrogate < Unicode::Surrogate_Low_Min) ||
                (low_surrogate > Unicode::Surrogate_Low_Max))
            {
                return {false, 0};
            }

            // Convert the high / low code point values to a UTF-32 value
            // (See: https://www.Unicode.org/faq/utf_bom.html#utf16-3).
            // NOTE: An alternative is this, but this takes more steps:
            //           character = ((character - 0xd800) << 10) +
            //                       (low_surrogate - 0xdc00) + 0x1'0000;
            character =
                (character << 10) + low_surrogate + Unicode::Surrogate_Offset;
        }

        // The following will produce the UTF-8 code point(s)
        // (See: https://www.rfc-editor.org/rfc/rfc3629#section-3)

        if (character <= 0x7f)
        {
            // 0nnnnnn
            *r++ = static_cast<std::uint8_t>(character & 0x7f);
            continue;
        }

        if (character <= 0x7ff)
        {
            // 110nnnnn 10nnnnnn
            *r++ = static_cast<std::uint8_t>(0xc0 | ((character >> 6) & 0x1f));
            *r++ = static_cast<std::uint8_t>(0x80 | ((character     ) & 0x3f));
            continue;
        }

        if (character <= 0xffff)
        {
            // 1110nnnn 10nnnnnn 10nnnnnn
            *r++ = static_cast<std::uint8_t>(0xe0 | ((character >> 12) & 0x0f));
            *r++ = static_cast<std::uint8_t>(0x80 | ((character >>  6) & 0x3f));
            *r++ = static_cast<std::uint8_t>(0x80 | ((character      ) & 0x3f));
            continue;
        }

        if (character <= 0x10'ffff)
        {
            // 11110nnn 10nnnnnn 10nnnnnn 10nnnnnn
            *r++ = static_cast<std::uint8_t>(0xf0 | ((character >> 18) & 0x07));
            *r++ = static_cast<std::uint8_t>(0x80 | ((character >> 12) & 0x3f));
            *r++ = static_cast<std::uint8_t>(0x80 | ((character >>  6) & 0x3f));
            *r++ = static_cast<std::uint8_t>(0x80 | ((character      ) & 0x3f));
            continue;
        }

        // We should never get to this point, as this would indicate an error
        return {false, 0};
    }

    return {true, static_cast<std::size_t>(std::distance(out.begin(), r))};
}

} // namespace Terra::CharUtil
