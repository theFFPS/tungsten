/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <vector>
#include <tungsten/nbt/nbt.hpp>
#include <tungsten/resource/resource_location.hpp>

namespace tungsten {

struct block_state_ref_t {
    resource_location_t m_name;
    std::map<std::string, std::string> m_properties;
};
struct structure_template_t {
    struct block_t {
        std::array<std::int32_t, 3> m_pos;
        std::int32_t m_state;
        std::optional<nbt_t> m_nbt;
    };
    struct entity_t {
        std::array<double, 3> m_pos;
        std::array<std::int32_t, 3> m_block_pos;
        nbt_t m_nbt;
    };

    std::int32_t m_data_version = 0;
    std::array<std::int32_t, 3> m_size {};
    std::vector<block_state_ref_t> m_palette;
    std::vector<std::vector<block_state_ref_t>> m_palettes;
    std::vector<block_t> m_blocks;
    std::vector<entity_t> m_entities;

    static structure_template_t parse(std::span<const std::uint8_t> bytes);
    static structure_template_t from_nbt(const nbt_t& root);
    const block_state_ref_t& state_of(const block_t& block) const;
};

}
