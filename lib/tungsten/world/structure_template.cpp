/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/world/structure_template.hpp>

#include <stdexcept>

namespace tungsten {

namespace {

[[noreturn]] void fail(const std::string& message) {
    throw std::runtime_error("structure_template_t error: " + message);
}
const nbt_t& require(const nbt_t& compound, const char* key) {
    const nbt_t* value = compound.find(key);
    if (value == nullptr) fail(std::string("missing required tag '") + key + "'");
    return *value;
}
std::vector<nbt_t> list_items(const nbt_t& tag, const char* key) {
    if (!tag.is_list()) fail(std::string("tag '") + key + "' must be a list");
    return tag.list().m_items;
}
std::array<std::int32_t, 3> read_int3(const nbt_t& tag, const char* key) {
    auto items = list_items(tag, key);
    if (items.size() != 3) fail(std::string("tag '") + key + "' must hold three values");
    return { static_cast<std::int32_t>(items[0].as_integer()), static_cast<std::int32_t>(items[1].as_integer()), static_cast<std::int32_t>(items[2].as_integer()) };
}
std::vector<block_state_ref_t> read_palette(const nbt_t& compound) {
    std::vector<block_state_ref_t> palette;
    for (const auto& entry : list_items(require(compound, "palette"), "palette")) {
        block_state_ref_t state;
        const nbt_t* name = entry.find("id");
        if (name == nullptr) name = entry.find("Name");
        if (name == nullptr) fail("palette entry has neither 'id' nor 'Name'");
        state.m_name = resource_location_t(name->string());
        const nbt_t* properties = entry.find("properties");
        if (properties == nullptr) properties = entry.find("Properties");
        if (properties != nullptr) {
            for (const auto& [name, value] : properties->compound()) state.m_properties[name] = value.string();
        }
        palette.push_back(std::move(state));
    }
    return palette;
}

}

structure_template_t structure_template_t::parse(std::span<const std::uint8_t> bytes) {
    try {
        return from_nbt(nbt_t::parse(bytes));
    } catch (const std::runtime_error& e) {
        std::string message = e.what();
        if (message.starts_with("structure_template_t error: ")) throw;
        fail(message);
    }
}
structure_template_t structure_template_t::from_nbt(const nbt_t& root) {
    if (!root.is_compound()) fail("the root tag must be a compound");
    structure_template_t result;
    if (const nbt_t* data_version = root.find("DataVersion")) result.m_data_version = static_cast<std::int32_t>(data_version->as_integer());
    result.m_size = read_int3(require(root, "size"), "size");
    for (auto dimension : result.m_size) if (dimension < 0) fail("negative size");

    if (const nbt_t* palettes = root.find("palettes")) {
        for (const auto& palette : list_items(*palettes, "palettes")) {
            nbt_t wrapper;
            wrapper.m_value = nbt_compound_t { { "palette", palette } };
            result.m_palettes.push_back(read_palette(wrapper));
        }
        if (result.m_palettes.empty()) fail("'palettes' is empty");
        result.m_palette = result.m_palettes.front();
    } else {
        result.m_palette = read_palette(root);
        result.m_palettes.push_back(result.m_palette);
    }

    for (const auto& entry : list_items(require(root, "blocks"), "blocks")) {
        block_t block;
        block.m_pos = read_int3(require(entry, "pos"), "pos");
        block.m_state = static_cast<std::int32_t>(require(entry, "state").as_integer());
        if (block.m_state < 0 || static_cast<std::size_t>(block.m_state) >= result.m_palette.size()) {
            fail("block state index " + std::to_string(block.m_state) + " is outside of the palette");
        }
        if (const nbt_t* block_nbt = entry.find("nbt")) block.m_nbt = *block_nbt;
        result.m_blocks.push_back(std::move(block));
    }
    if (const nbt_t* entities = root.find("entities")) {
        for (const auto& entry : list_items(*entities, "entities")) {
            entity_t entity;
            auto pos = list_items(require(entry, "pos"), "pos");
            if (pos.size() != 3) fail("entity 'pos' must hold three values");
            for (std::size_t i = 0; i < 3; ++i) entity.m_pos[i] = pos[i].as_floating();
            entity.m_block_pos = read_int3(require(entry, "blockPos"), "blockPos");
            entity.m_nbt = require(entry, "nbt");
            result.m_entities.push_back(std::move(entity));
        }
    }
    return result;
}
const block_state_ref_t& structure_template_t::state_of(const block_t& block) const {
    if (block.m_state < 0 || static_cast<std::size_t>(block.m_state) >= m_palette.size()) {
        throw std::runtime_error("structure_template_t::state_of error: palette index out of range");
    }
    return m_palette[static_cast<std::size_t>(block.m_state)];
}

}
