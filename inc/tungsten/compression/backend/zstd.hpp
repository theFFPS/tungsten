#pragma once

#include <cstdint>
#include <vector>

namespace tungsten {
namespace backend {

struct zstd_compression_t {
    static std::vector<uint8_t> compress(const uint8_t* src, size_t len, int level);
    static std::vector<uint8_t> decompress(const uint8_t* src, size_t src_len, size_t out_len);
};

}
}
