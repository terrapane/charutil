/*
 *  character_utilities.cpp
 *
 *  Copyright (C) 2024
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

#include <limits>
#include <algorithm>
#include <terra/charutil/character_utilities.h>

namespace Terra::CharUtil
{

namespace Unicode
{

// Largest valid Unicode character
constexpr std::uint32_t Maximum_Character_Value = 0x10'ffff;

// Maximum unicode character in the BMP
constexpr std::uint32_t Maximum_BMP_Value = 0xffff;

// Surrogates are within the following range
// (See: https://en.wikipedia.org/wiki/UTF-16#U+D800_to_U+DFFF_(surrogates))
constexpr std::uint32_t Surrogate_High_Min = 0xd800;
[[maybe_unused]] constexpr std::uint32_t Surrogate_High_Max = 0xdbff;
constexpr std::uint32_t Surrogate_Low_Min = 0xdc00;
constexpr std::uint32_t Surrogate_Low_Max = 0xdfff;

// Values used in parsing or creating surrogate pairs
// (See: https://www.Unicode.org/faq/utf_bom.html#utf16-3)
constexpr std::uint32_t Lead_Offset = 0xd800 - (0x1'0000 >> 10);
constexpr std::uint32_t Surrogate_Offset = 0xfca0'2400;
// Surrogate_Offset = 0x1'0000 - (0xd800 << 10) - 0xdc00;
// Pre-computed to avoid compiler warnings about
} // namespace Unicode

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
 *  ConvertUTF8ToUTF16()
 *
 *  Description:
 *      This function will take a span of octets in UTF-8 format and convert
 *      them to UTF-16 format.  This function will not insert byte-order-mark
 *      (BOM) octets.  The endianness is specified via the third parameter.
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
 *          Store the UTF-16 characters in little endian order?
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
    if (out.size() < in.size() * 2) return {false, 0};

    // Assign the output pointer
    std::uint8_t *p = out.data();

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

                    // Adjust the pointer p
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

                    // Adjust the pointer p
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

    return {true, static_cast<std::size_t>(p - out.data())};
}

/*
 *  ConvertUTF16ToUTF8()
 *
 *  Description:
 *      This function will take a span of octets in UTF-16 format and convert
 *      them to UTF-8 format.  The UTF-16 octets must NOT have a
 *      byte-order-mark (BOM) at the start.  The endianness is indicated
 *      via the third argument.
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
 *          Are the UTF-16 characters in little endian order?
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
    // Get the length of the UTF-16-encoded string
    std::size_t pw_length = in.size();

    // If the input is zero length, so is the output
    if (pw_length == 0) return {true, 0};

    // UTF-16 always has an even number of octets, so verify that is the case
    if ((pw_length & 1) != 0) return {false, 0};

    // Reject UTF-16 strings that are to long
    if (pw_length > Max_UTF16_String) return {false, 0};

    // If the output span is an insufficient size, return an error (1.5x size)
    if (out.size() < (pw_length + (pw_length >> 1))) return {false, 0};

    // Assign the input and output pointers
    const std::uint8_t *p = in.data();
    const std::uint8_t *q = in.data() + pw_length;
    std::uint8_t *r = out.data();

    // Iterate over the input span
    while (p < q)
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

        // Advance the pointer to the next character (or surrogate)
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
            if (p >= q) return {false, 0};

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

            // Advance the pointer to the next character (for next iteration)
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

    return {true, static_cast<std::size_t>(r - out.data())};
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
bool IsUTF8Valid(std::span<const std::uint8_t> octets)
{
    std::size_t expected_utf8_remaining{};      // Number of UTF-8 octets left
    std::uint32_t wide_character{};             // UTF-32 character

    // If the input is zero length, so is the output
    if (octets.empty()) return true;

    // Iterate over the span of octets
    for (std::uint8_t octet : octets)
    {
        // Handle subsequent UTF-8 octets
        if (expected_utf8_remaining > 0)
        {
            // Expecting a 10xxxxxx octet
            if ((octet & 0xc0) != 0x80) return false;

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
                    return false;
                }

                // Ensure the character code is not within the surrogate range
                if ((wide_character >= Unicode::Surrogate_High_Min) &&
                    (wide_character <= Unicode::Surrogate_Low_Max))
                {
                    return false;
                }
            }

            // Multi-octet sequence is valid, so continue
            continue;
        }

        // Single ASCII character?
        if (octet <= 0x7f) continue;

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
        return false;
    }

    // If there are other octets expected, conversion was successful
    return expected_utf8_remaining == 0;
}

} // namespace Terra::CharUtil
