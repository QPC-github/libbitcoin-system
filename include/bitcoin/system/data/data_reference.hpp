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
#ifndef LIBBITCOIN_SYSTEM_DATA_DATA_REFERENCE_HPP
#define LIBBITCOIN_SYSTEM_DATA_DATA_REFERENCE_HPP

#include <array>
#include <string>
#include <vector>
/// DELETEMENOW
#include <bitcoin/system/data/data_array.hpp>
#include <bitcoin/system/data/data_chunk.hpp>
#include <bitcoin/system/data/data_slice.hpp>
#include <bitcoin/system/define.hpp>

namespace libbitcoin {
namespace system {

/// Lifetime:
/// No prvalues are accepted, allowing extended lifetime.
/// See discussion of lifetime considerations in data_slice comments.
/// The slab does not extend the lifetime of any rvalue passed to it.
/// A data_reference can be used as a data_slice, but a data_slice cannot be
/// used as a data_reference, which is why the inheritance is reductive.
/// Deleting prvalue constructors prevents passage or materialization of
/// temporaries.

/// Downsizable but otherwise const iterable wrapper for const buffer.
/// Identical to data_slice except does not accept prvalues.
/// Not a substitute for move overrides or containment.
/// Accepts any sizeof(T) == 1 type as a "byte" and emits uint8_t.
/// [] iteration past end is safe and returns zeros.
/// Negative size construction yields a valid empty object.
class BC_API data_reference
  : public data_slice
{
public:
    /// Empty reference.
    data_reference() NOEXCEPT;

    /// Defaults.
    data_reference(data_reference&&) = default;
    data_reference(const data_reference&) = default;
    data_reference& operator=(data_reference&&) = default;
    data_reference& operator=(const data_reference&) = default;
    ~data_reference() = default;

    /// data_slice construction.
    data_reference(const data_slice& data) NOEXCEPT;

    /// std::string constructor (casts char to uint8_t).
    data_reference(const std::string& text) NOEXCEPT;

    /// data_chunk constructor.
    data_reference(const data_chunk& data) NOEXCEPT;

    /// data_array constructor.
    template <size_type Size>
    data_reference(const data_array<Size>& data) NOEXCEPT;

    /// Byte array constructor (casts Byte to uint8_t).
    template <size_type Size, typename Byte, if_one_byte<Byte> = true>
    data_reference(const std::array<Byte, Size>& data) NOEXCEPT;

    /// Byte vector constructor (casts Byte to uint8_t).
    template <typename Byte, if_one_byte<Byte> = true>
    data_reference(const std::vector<Byte>& data) NOEXCEPT;

    /// Byte iterators constructor (casts to uint8_t).
    template <typename Iterator>
    data_reference(const Iterator& begin, const Iterator& end) NOEXCEPT;

    /// Byte pointer to const constructor (casts Byte to uint8_t).
    template <typename Byte, if_one_byte<Byte> = true>
    data_reference(const Byte* begin, const Byte* end) NOEXCEPT;
};

} // namespace system
} // namespace libbitcoin

#include <bitcoin/system/impl/data/data_reference.ipp>

#endif
