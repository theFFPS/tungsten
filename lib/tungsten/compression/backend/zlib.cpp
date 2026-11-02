#include <tungsten/compression/backend/zlib.hpp>
#include <stdexcept>
#include <zlib-ng.h>

namespace tungsten {
namespace backend {

std::vector<uint8_t> zlib_compression_t::compress(const uint8_t* src, size_t len, int level) {
    size_t compressed_size = zng_compressBound(len);
    std::vector<uint8_t> compressed_text (compressed_size);
    if (zng_compress2(compressed_text.data(), &compressed_size, src, len, level) != Z_OK) {
        throw std::runtime_error("[compression backend] zlib_compression_t::compress error: zlib-ng failed to compress");
    }
    compressed_text.resize(compressed_size);
    return compressed_text;
}
std::vector<uint8_t> zlib_compression_t::decompress(const uint8_t* src, size_t src_len, size_t out_len) {
    std::vector<uint8_t> plain_text (out_len);
    size_t compressed_size = src_len;
    if (zng_uncompress2(plain_text.data(), &out_len, src, &compressed_size) != Z_OK) {
        throw std::runtime_error("[compression backend] zlib_compression_t::decompress error: zlib-ng failed to uncompress");
    }
    plain_text.resize(out_len);
    return plain_text;
}

}
}
