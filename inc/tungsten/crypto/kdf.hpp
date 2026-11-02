#pragma once

#include <vector>
#include <span>
#include <cstdint>
#include <tungsten/util/allocator.hpp>

namespace tungsten {

std::vector<uint8_t, secure_allocator_t<uint8_t>> hkdf_sha3_512(std::span<const uint8_t> data, std::span<const uint8_t> salt, std::span<const uint8_t> info, size_t olen);
std::vector<uint8_t, secure_allocator_t<uint8_t>> argon2id(std::span<const uint8_t> password, std::span<const uint8_t> salt, size_t olen, size_t memcost, uint32_t iter);

}
