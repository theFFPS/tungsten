/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

module;
#include <zlib-ng.h>
#include <zstd.h>

module tungsten.compression;

import std;

namespace tungsten {

namespace backend {

std::vector<std::uint8_t> zlib_compression_t::compress(const std::uint8_t* src, std::size_t len, int level) {
    std::size_t compressed_size = zng_compressBound(len);
    std::vector<std::uint8_t> compressed_text (compressed_size);
    if (zng_compress2(compressed_text.data(), &compressed_size, src, len, level) != Z_OK) {
        throw std::runtime_error("[compression backend] zlib_compression_t::compress error: zlib-ng failed to compress");
    }
    compressed_text.resize(compressed_size);
    return compressed_text;
}
std::vector<std::uint8_t> zlib_compression_t::decompress(const std::uint8_t* src, std::size_t src_len, std::size_t out_len) {
    std::vector<std::uint8_t> plain_text (out_len);
    std::size_t compressed_size = src_len;
    if (zng_uncompress2(plain_text.data(), &out_len, src, &compressed_size) != Z_OK) {
        throw std::runtime_error("[compression backend] zlib_compression_t::decompress error: zlib-ng failed to uncompress");
    }
    plain_text.resize(out_len);
    return plain_text;
}

std::vector<std::uint8_t> zlib_compression_t::gzip_compress(const std::uint8_t* src, std::size_t len, int level) {
    zng_stream stream {};
    if (zng_deflateInit2(&stream, level, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        throw std::runtime_error("[compression backend] zlib_compression_t::gzip_compress error: zlib-ng failed to initialize");
    }
    std::vector<std::uint8_t> out (len / 2 + 64);
    stream.next_in = src;
    stream.avail_in = static_cast<std::uint32_t>(len);
    int status = Z_OK;
    while (status == Z_OK) {
        if (stream.total_out == out.size()) out.resize(out.size() * 2);
        stream.next_out = out.data() + stream.total_out;
        stream.avail_out = static_cast<std::uint32_t>(out.size() - stream.total_out);
        status = zng_deflate(&stream, Z_FINISH);
    }
    std::size_t total = stream.total_out;
    zng_deflateEnd(&stream);
    if (status != Z_STREAM_END) {
        throw std::runtime_error("[compression backend] zlib_compression_t::gzip_compress error: zlib-ng failed to compress");
    }
    out.resize(total);
    return out;
}
std::vector<std::uint8_t> zlib_compression_t::gzip_decompress(const std::uint8_t* src, std::size_t src_len, std::size_t size_hint) {
    zng_stream stream {};
    if (zng_inflateInit2(&stream, 47) != Z_OK) {
        throw std::runtime_error("[compression backend] zlib_compression_t::gzip_decompress error: zlib-ng failed to initialize");
    }
    std::vector<std::uint8_t> out (size_hint != 0 ? size_hint : std::max<std::size_t>(src_len * 4, 256));
    stream.next_in = src;
    stream.avail_in = static_cast<std::uint32_t>(src_len);
    int status = Z_OK;
    while (status == Z_OK) {
        if (stream.total_out == out.size()) out.resize(out.size() * 2);
        stream.next_out = out.data() + stream.total_out;
        stream.avail_out = static_cast<std::uint32_t>(out.size() - stream.total_out);
        status = zng_inflate(&stream, Z_NO_FLUSH);
    }
    std::size_t total = stream.total_out;
    zng_inflateEnd(&stream);
    if (status != Z_STREAM_END) {
        throw std::runtime_error("[compression backend] zlib_compression_t::gzip_decompress error: zlib-ng failed to uncompress (corrupt or truncated stream)");
    }
    out.resize(total);
    return out;
}

std::vector<std::uint8_t> zstd_compression_t::compress(const std::uint8_t* src, std::size_t len, int level) {
    std::size_t compressed_size = ZSTD_compressBound(len);
    std::vector<std::uint8_t> compressed_text (compressed_size);
    compressed_size = ZSTD_compress(compressed_text.data(), compressed_size, src, len, level);
    compressed_text.resize(compressed_size);
    return compressed_text;

}
std::vector<std::uint8_t> zstd_compression_t::decompress(const std::uint8_t* src, std::size_t src_len, std::size_t out_len) {
    std::vector<std::uint8_t> plain_text (out_len);
    ZSTD_decompress(plain_text.data(), out_len, src, src_len);
    return plain_text;
}

}

std::vector<std::uint8_t> compression_t::compress(compression_type_t type, const std::uint8_t* src, std::size_t len, int level) {
    switch (type) {
    case tungsten::compression_type_t::zlib: return backend::zlib_compression_t::compress(src, len, level);
    case tungsten::compression_type_t::zstd: return backend::zstd_compression_t::compress(src, len, level);
    case tungsten::compression_type_t::gzip: return backend::zlib_compression_t::gzip_compress(src, len, level);
    default: return std::vector<std::uint8_t>(src, src + len);
    }
}
std::vector<std::uint8_t> compression_t::decompress(compression_type_t type, const std::uint8_t* src, std::size_t src_len, std::size_t out_len) {
    switch (type) {
    case tungsten::compression_type_t::zlib: return backend::zlib_compression_t::decompress(src, src_len, out_len);
    case tungsten::compression_type_t::zstd: return backend::zstd_compression_t::decompress(src, src_len, out_len);
    case tungsten::compression_type_t::gzip: return backend::zlib_compression_t::gzip_decompress(src, src_len, out_len);
    default: return std::vector<std::uint8_t>(src, src + src_len);
    }
}

}
