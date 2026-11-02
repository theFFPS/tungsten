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
