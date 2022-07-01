/**
 * Copyright (c) 2011-2022 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_SYSTEM_MACHINE_NUMBER_IPP
#define LIBBITCOIN_SYSTEM_MACHINE_NUMBER_IPP

#include <algorithm>
/// DELETECSTDDEF
/// DELETECSTDINT
#include <iterator>
/// DELETEMENOW
/////// DELETEMENOW
#include <bitcoin/system/data/data.hpp>
#include <bitcoin/system/define.hpp>
#include <bitcoin/system/math/math.hpp>
#include <bitcoin/system/serial/serial.hpp>

namespace libbitcoin {
namespace system {
namespace machine {
namespace number {

// ****************************************************************************
// CONSENSUS:
// Due to compression, a leading sign byte is required if high bit is set.
// This produces two zero representations (+/-0). The subsequent byte-based
// overflow constraint thereby restricts the stack's 64-bit integer domain by
// one value. An implementation that either avoided vectorization or vector
// compression in the first place, or later implemented the overflow guard over
// the converted integeral would have avoided this seam. These were both
// premature optimizations, as a variant stack is more efficient than variably-
// lengthed byte vector integral storage. The resulting integer domain
// reduction is captured by the is_overflow functions below.
// ****************************************************************************

constexpr uint8_t positive_sign_byte = 0x00;
constexpr uint8_t negative_sign_byte = to_negated(positive_sign_byte);

// integer
// ----------------------------------------------------------------------------
// C++20: constexpr.

template <size_t Size>
inline bool integer<Size>::from_integer(Integer& out, int64_t vary) NOEXCEPT
{
    out = possible_narrow_cast<Integer>(vary);
    return !is_overflow(vary);
}

template <size_t Size>
inline bool integer<Size>::from_chunk(Integer& out,
    const data_chunk& vary) NOEXCEPT
{
    out = 0;

    // Just an optimization.
    if (strict_zero(vary))
        return true;

    if (is_overflow(vary))
        return false;

    out = from_little_endian<Integer>(vary);

    // Restore sign from indication.
    if (is_negated(vary.back()))
        out = to_unnegated(out);

    return true;
}

// protected
template <size_t Size>
inline bool integer<Size>::strict_zero(const data_chunk& vary) NOEXCEPT
{
    return vary.empty();
}

// protected
template <size_t Size>
inline bool integer<Size>::is_overflow(const data_chunk& vary) NOEXCEPT
{
    return vary.size() > Size;
}

// protected
template <size_t Size>
inline bool integer<Size>::is_overflow(int64_t value) NOEXCEPT
{
    return is_limited(value, bitcoin_min<Size>(), bitcoin_max<Size>());
}

// chunk
// ----------------------------------------------------------------------------
// Minimally-sized byte encoding, with extra allocated byte if negated.
// absolute(minimum<int64_t>) is guarded by the presumption of int32 ops.

inline data_chunk chunk::from_bool(bool vary) NOEXCEPT
{
    return { bc::to_int<uint8_t>(vary) };
}

inline data_chunk chunk::from_integer(int64_t vary) NOEXCEPT
{
    // Just an optimization.
    if (is_zero(vary))
        return {};

    // absolute is unsafe but this is guarded by stack int32_t limitation.
    const auto value = absolute(vary);
    const auto negated = is_negated(value);
    const auto negative = is_negative(vary);

    auto bytes = to_little_endian_size(value, to_int(negated));

    // Indicate the sign.
    if (negated && negative)
        bytes.push_back(negative_sign_byte);
    else if (negated)
        bytes.push_back(positive_sign_byte);
    else if (negative)
        bytes.back() = to_negated(bytes.back());

    return bytes;
}

// boolean
// ----------------------------------------------------------------------------
// C++20: constexpr.

template <size_t Size>
inline signed_type<Size> boolean::to_integer(bool vary) NOEXCEPT
{
    // The cast can safely be ignored, which is why Size is defaulted.
    return bc::to_int<signed_type<Size>>(vary);
}

inline bool boolean::from_chunk(const data_chunk& vary) NOEXCEPT
{
    // An optimization, also guards vector empty.
    if (strict_false(vary))
        return false;

    // Boolean is not overflow constrained (any length of bytes is valid).
    ////if (is_overflow(vary))
    ////    return false;

    if (!is_sign_byte(vary.back()))
        return true;

    // A logical zero is any +/- sequence of zero bytes (sign excluded above).
    return std::any_of(vary.begin(), std::prev(vary.end()), is_nonzero<uint8_t>);

    // any_of optimizes by eliminating this conversion, allocation, and copy.
    ////return bc::to_bool(from_little_endian<uintx>(vary));
}

inline bool boolean::strict_from_chunk(const data_chunk& vary) NOEXCEPT
{
    // Strict bool tests for integral false/zero, or a single empty byte.
    return strict_false(vary);
}

constexpr bool boolean::to_bool(int64_t vary) NOEXCEPT
{
    // Boolean is not overflow constrained.
    return bc::to_bool(vary);
}

// protected
inline bool boolean::strict_false(const data_chunk& vary) NOEXCEPT
{
    return vary.empty();
}

// protected
constexpr bool boolean::is_sign_byte(uint8_t byte) NOEXCEPT
{
    return (byte == positive_sign_byte) || (byte == negative_sign_byte);
}

} // namespace number
} // namespace machine
} // namespace system
} // namespace libbitcoin

#endif
