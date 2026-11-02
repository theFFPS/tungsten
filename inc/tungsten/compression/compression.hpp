#pragma once

#include <cstdint>
#include <vector>

namespace tungsten {

enum class compression_type_t : uint8_t {
    none,
    zstd,
    zlib
};

struct compression_t {
    static std::vector<uint8_t> compress(compression_type_t type, const uint8_t* src, size_t len, int level);
    static std::vector<uint8_t> decompress(compression_type_t type, const uint8_t* src, size_t src_len, size_t out_len);
};

}
