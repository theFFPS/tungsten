/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <variant>
#include <vector>
#include <glaze/glaze.hpp>

namespace tungsten {

enum class nbt_type_t : std::uint8_t {
    end = 0, byte = 1, short_ = 2, int_ = 3, long_ = 4, float_ = 5, double_ = 6,
    byte_array = 7, string = 8, list = 9, compound = 10, int_array = 11, long_array = 12
};

struct nbt_t;
using nbt_compound_t = std::map<std::string, nbt_t>;
struct nbt_list_t {
    nbt_type_t m_element_type = nbt_type_t::end;
    std::vector<nbt_t> m_items;
};

// a binary NBT value (https://minecraft.wiki/w/NBT_format)
struct nbt_t {
    using value_t = std::variant<
        std::int8_t, std::int16_t, std::int32_t, std::int64_t, float, double,
        std::vector<std::int8_t>, std::string, nbt_list_t, nbt_compound_t, std::vector<std::int32_t>, std::vector<std::int64_t>
    >;
    value_t m_value = nbt_compound_t {};

    nbt_type_t type() const noexcept;

    bool is_compound() const noexcept { return std::holds_alternative<nbt_compound_t>(m_value); }
    bool is_list() const noexcept { return std::holds_alternative<nbt_list_t>(m_value); }
    const nbt_compound_t& compound() const;
    const nbt_list_t& list() const;
    const std::string& string() const;

    // compound member lookup, nullptr when this is not a compound or the key is missing
    const nbt_t* find(std::string_view key) const noexcept;
    // numeric accessors accept any integer tag; they throw std::runtime_error on a type mismatch
    std::int64_t as_integer() const;
    double as_floating() const;

    // `bytes` may be gzip / zlib compressed (detected automatically) or raw. The root is a named compound tag.
    static nbt_t parse(std::span<const std::uint8_t> bytes, std::string* root_name = nullptr);
    // representation used by SNBT-ish tooling: numbers keep their value, arrays become arrays, compounds become objects
    glz::generic to_json() const;
};

}
