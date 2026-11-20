/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/util/encoding.hpp>
#include <stdexcept>
#include <algorithm>

#include <tungsten/crypto.hpp>
import tungsten.crypto;

namespace tungsten {

std::string base64_t::encode(std::span<const uint8_t> in)

std::vector<uint8_t> base64_t::decode(std::string_view in)

std::string base58_t::encode(std::span<const uint8_t> in)

std::vector<uint8_t> base58_t::decode(std::string_view in)

std::string to_hex_str(std::span<const uint8_t> in)

}
