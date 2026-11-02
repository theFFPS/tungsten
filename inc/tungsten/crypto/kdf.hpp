/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <vector>
#include <span>
#include <cstdint>
#include <tungsten/util/allocator.hpp>

namespace tungsten {

std::vector<uint8_t, secure_allocator_t<uint8_t>> hkdf_sha3_512(std::span<const uint8_t> data, std::span<const uint8_t> salt, std::span<const uint8_t> info, size_t olen);
std::vector<uint8_t, secure_allocator_t<uint8_t>> argon2id(std::span<const uint8_t> password, std::span<const uint8_t> salt, size_t olen, size_t memcost, uint32_t iter);

}
