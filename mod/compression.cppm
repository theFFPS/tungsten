/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

export module tungsten.compression;

import std;

export namespace tungsten {

namespace backend {

struct zlib_compression_t {
    static std::vector<std::uint8_t> compress(const std::uint8_t* src, std::size_t len, int level);
    static std::vector<std::uint8_t> decompress(const std::uint8_t* src, std::size_t src_len, std::size_t out_len);

    static std::vector<std::uint8_t> gzip_compress(const std::uint8_t* src, std::size_t len, int level);
    static std::vector<std::uint8_t> gzip_decompress(const std::uint8_t* src, std::size_t src_len, std::size_t size_hint = 0);
};

struct zstd_compression_t {
    static std::vector<std::uint8_t> compress(const std::uint8_t* src, std::size_t len, int level);
    static std::vector<std::uint8_t> decompress(const std::uint8_t* src, std::size_t src_len, std::size_t out_len);
};

}

enum class compression_type_t : std::uint8_t {
    none,
    zstd,
    zlib,
    gzip
};

struct compression_t {
    static std::vector<std::uint8_t> compress(compression_type_t type, const std::uint8_t* src, std::size_t len, int level);
    static std::vector<std::uint8_t> decompress(compression_type_t type, const std::uint8_t* src, std::size_t src_len, std::size_t out_len);
};

}
