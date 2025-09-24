/*
 *  is_valid_utf8.cpp
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

#include <terra/charutil/character_utilities.h>
#include "unicode_constants.h"

namespace Terra::CharUtil
{

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
        // Invalid octet values
        if ((octet == 0xc0) || (octet == 0xc1)) return false;
        if (octet >= 0xF5) return false;

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

    // If there are no other octets expected, it is a valid UTF-8 string
    return expected_utf8_remaining == 0;
}

} // namespace Terra::CharUtil
