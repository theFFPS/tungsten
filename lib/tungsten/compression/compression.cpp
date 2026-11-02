/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/compression/compression.hpp>
#include <tungsten/compression/backend/zlib.hpp>
#include <tungsten/compression/backend/zstd.hpp>

namespace tungsten {

std::vector<uint8_t> compression_t::compress(compression_type_t type, const uint8_t* src, size_t len, int level) {
    switch (type) {
    case tungsten::compression_type_t::zlib: return backend::zlib_compression_t::compress(src, len, level);
    case tungsten::compression_type_t::zstd: return backend::zstd_compression_t::compress(src, len, level);
    default: return std::vector<uint8_t>(src, src + len);
    }
}
std::vector<uint8_t> compression_t::decompress(compression_type_t type, const uint8_t* src, size_t src_len, size_t out_len) {
    switch (type) {
    case tungsten::compression_type_t::zlib: return backend::zlib_compression_t::decompress(src, src_len, out_len);
    case tungsten::compression_type_t::zstd: return backend::zstd_compression_t::decompress(src, src_len, out_len);
    default: return std::vector<uint8_t>(src, src + src_len);
    }
}

}
