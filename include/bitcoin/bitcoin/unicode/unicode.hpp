﻿/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_UNICODE_HPP
#define LIBBITCOIN_UNICODE_HPP

#include <cstddef>
#include <iostream>
#include <string>
#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/utility/data.hpp>

// Regarding Unicode design for Windows:
//
// Windows and other environments, such as Java, that supported Unicode prior
// to the advent of utf8 utilize 16 bit characters. These are typically encoded
// as wchar_t in C++. Unicode no longer fits in 16 bits and as such these
// implementations now require variable length character encoding just as utf8.
//
// Libbitcoin embraces the "utf8 everywhere" design: http://utf8everywhere.org
// The objective is to use utf8 as the canonical string encoding, pushing
// wchar_t translation to the edge (stdio, argv, O/S and external API calls).
// The macro BC_USE_LIBBITCOIN_MAIN does most of the heavy lifting to ensure
// that stdio and argv is configured for utf8. The 'to_utf' functions are
// provided for API translations.

// Regarding Unicode source files in VC++ builds:
//
// Don't use #pragma execution_character_set("utf-8") in Windows.
// This instruction causes sources to be explicitly interpreted as UTF8.
// However this technique improperly encodes literals ("フ" for example).
// Instead use non-BOM UTF encoded files. To do this use "save as..."
// utf8 without a "signature" (BOM) in the Visual Studio save dialog.
// This is the typical file format for C++ sources in other environments.

// Regarding Unicode in console applications:
//
// BC_USE_LIBBITCOIN_MAIN should be declared prior to bc::main() in a console
// application. This enables Unicode argument and environment processing in 
// Windows. This macro implements main() and forwards to bc::main(), which 
// should be implemented as if it was main() with the expectation that argv 
// is utf8.
//
// Do not use std::cout|std::cerr|std::cin (aborts on assertion):
// std::cout << "text";
// std::cerr << "text";
// std::string text;
// std::cin >> text;
//
// Do not use the L qualifier when source is UTF-8 w/out BOM (mangles output):
// const auto utf16 = L"acción.кошка.日本国";
// std::wcout << utf16;

// Regarding use of boost:
//
// When working with boost and utf8 narrow characters on Windows the thread
// must be configured for utf8. When working with boost::filesystem::path the
// static path object must be imbued with the utf8 locale or paths will be 
// incorrectly translated.

#ifdef _WIN32
    #define BOOST_HAS_ICU
#endif
#ifdef BOOST_HAS_ICU
    #define BC_HAS_ICU
#endif

#define BC_LOCALE_BACKEND "icu"
#define BC_LOCALE_UTF8 "en_US.UTF8"

#ifdef _MSC_VER
    #include <locale>
    #include <boost/filesystem.hpp>
    #include <boost/locale.hpp>
    #include <fcntl.h>
    #include <io.h>
    #include <windows.h>
    #define BC_USE_LIBBITCOIN_MAIN \
        namespace libbitcoin { \
        int main(int argc, char* argv[]); \
        } \
        \
        int wmain(int argc, wchar_t* argv[]) \
        { \
            _setmode(_fileno(stdin), _O_U8TEXT); \
            _setmode(_fileno(stdout), _O_U8TEXT); \
            _setmode(_fileno(stderr), _O_U8TEXT); \
            \
            using namespace libbitcoin; \
            boost::locale::generator locale; \
            std::locale::global(locale(BC_LOCALE_UTF8)); \
            boost::filesystem::path::imbue(std::locale()); \
            \
            auto variables = to_utf8(_wenviron); \
            environ = reinterpret_cast<char**>(variables.data()); \
            \
            auto arguments = to_utf8(argc, argv); \
            auto args = reinterpret_cast<char**>(arguments.data()); \
            \
            return libbitcoin::main(argc, args); \
        }
#else
    #define BC_USE_LIBBITCOIN_MAIN \
        namespace libbitcoin { \
        int main(int argc, char* argv[]); \
        } \
        \
        int main(int argc, char* argv[]) \
        { \
            return libbitcoin::main(argc, argv); \
        }
#endif

namespace libbitcoin {

/**
 * Use bc::cin in place of std::cin.
 */
extern std::istream& cin;

/**
 * Use bc::cout in place of std::cout.
 */
extern std::ostream& cout;

/**
 * Use bc::cerr in place of std::cerr.
 */
extern std::ostream& cerr;

#ifdef BOOST_HAS_ICU

/**
 * Normalize a string value using nfkd normalization.
 * This function requires the ICU dependency.
 * @param[in]  value  The value to normalize.
 * @return            The normalized value.
 */
BC_API std::string to_normal_form(const std::string& value);

#endif

/**
 * Convert wide environment vector to utf8 environment vector.
 * Caller should assign buffer and set result to environ as:
 * environ = reinterpret_cast<char**>(&buffer[0])
 * @param[in]  environment  The wide environment variables vector.
 * @return                  A buffer holding the narrow version of environment.
 */
BC_API data_chunk to_utf8(wchar_t* environment[]);

/**
 * Convert wide argument vector to utf8 argument vector.
 * Caller should assign buffer and reinterpret result as:
 * auto args = reinterpret_cast<char**>(&buffer[0])
 * @param[in]  argc  The number of elements in argv.
 * @param[in]  argv  The wide command line arguments.
 * @return           A buffer holding the narrow version of argv.
 */
BC_API data_chunk to_utf8(int argc, wchar_t* argv[]);

/**
 * Convert a wide (presumed UTF16) array to wide (UTF8/char).
 * @param[in]  out        The converted narrow array.
 * @param[in]  out_bytes  The allocated number of bytes in 'out'.
 * @param[in]  in         The wide character array to convert.
 * @param[in]  in_chars   The number of 'in' wide characters to convert.
 * @return                The number of bytes converted.
 */
BC_API size_t to_utf8(char out[], size_t out_bytes, const wchar_t in[],
    size_t in_chars);

/**
 * Convert a wide (presumed UTF16) string to narrow (UTF8/char).
 * @param[in]  wide  The utf16 string to convert.
 * @return           The resulting utf8 string.
 */
BC_API std::string to_utf8(const std::wstring& wide);

/**
 * Convert a narrow (presumed UTF8) array to wide (UTF16/wchar_t).
 * This is designed for buffering, where the narrow array may have been
 * truncated in the middle of a multiple byte character. The terminating
 * offset is returned in the 'unread' out parameter.
 * @param[in]  out        The converted wide array.
 * @param[in]  out_chars  The allocated number of wide characters in 'out'.
 * @param[in]  in         The narrow array to convert.
 * @param[in]  in_bytes   The number of 'in' bytes to convert.
 * @param[in]  truncated  The number of 'in' bytes [0..3] that were truncated.
 * @return                The number of characters converted.
 */
BC_API size_t to_utf16(wchar_t out[], size_t out_chars, const char in[],
    size_t in_bytes, uint8_t& truncated);

/**
 * Convert a narrow (presumed UTF8) string to wide (UTF16/wchar_t).
 * @param[in]  narrow  The utf8 string to convert.
 * @return             The resulting utf16 string.
 */
BC_API std::wstring to_utf16(const std::string& narrow);

} // namespace libbitcoin

#endif
