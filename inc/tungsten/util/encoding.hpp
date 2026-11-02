#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string_view>

namespace tungsten {

struct base64_t {
    static std::string encode(std::span<const uint8_t> in);
    static std::vector<uint8_t> decode(std::string_view in);
};
struct base58_t {
    static std::string encode(std::span<const uint8_t> in);
    static std::vector<uint8_t> decode(std::string_view in);
};
std::string to_hex_str(std::span<const uint8_t> in);

}
