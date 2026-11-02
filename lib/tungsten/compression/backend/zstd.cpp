#include <tungsten/compression/backend/zstd.hpp>
#include <zstd.h>

namespace tungsten {
namespace backend {

std::vector<uint8_t> zstd_compression_t::compress(const uint8_t* src, size_t len, int level) {
    size_t compressed_size = ZSTD_compressBound(len);
    std::vector<uint8_t> compressed_text (compressed_size);
    compressed_size = ZSTD_compress(compressed_text.data(), compressed_size, src, len, level);
    compressed_text.resize(compressed_size);
    return compressed_text;

}
std::vector<uint8_t> zstd_compression_t::decompress(const uint8_t* src, size_t src_len, size_t out_len) {
    std::vector<uint8_t> plain_text (out_len);
    ZSTD_decompress(plain_text.data(), out_len, src, src_len);
    return plain_text;
}

}
}
