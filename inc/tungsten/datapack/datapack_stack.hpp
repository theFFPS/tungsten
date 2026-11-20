/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <tungsten/datapack/datapack.hpp>
#include <tungsten/datapack/load_report.hpp>

namespace tungsten {

struct resolved_file_t {
    std::shared_ptr<datapack_t> m_pack;
    datapack_file_t m_file;

    resource_location_t id(std::string_view extension) const;
};

// Ordered list of data packs, first = lowest priority, last = highest. Implements the layering rules of the game:
//  - a file present in several packs is taken from the last pack (override), tags are merged instead
//  - the "filter" section of a pack hides matching files of all packs BELOW it (never of itself)
//  - overlays of a pack are applied over the pack's own files
struct datapack_stack_t {
private:
    struct layer_t {
        std::size_t m_pack_index;
        datapack_file_t m_file;
    };
    using path_index_t = std::map<std::string, std::vector<layer_t>>; // path -> layers sorted by pack index

    std::vector<std::shared_ptr<datapack_t>> m_packs;
    pack_version_t m_version = current_data_pack_version;
    mutable std::map<std::string, path_index_t> m_index; // namespace -> files
    mutable bool m_index_valid = false;

    void build_index() const;
    const std::map<std::string, path_index_t>& index() const;
public:
    datapack_stack_t() = default;
    explicit datapack_stack_t(pack_version_t version) : m_version(version) {}

    const std::vector<std::shared_ptr<datapack_t>>& packs() const noexcept { return m_packs; }
    const pack_version_t& version() const noexcept { return m_version; }

    // appends on top; packs which are incompatible with the version are still added (see load_report in the loader)
    void add(std::shared_ptr<datapack_t> pack);
    void clear();
    bool remove(std::string_view name);

    // builds the file index now; afterwards all const members are safe to call from several threads
    void prepare() const { index(); }
    std::set<std::string> namespaces() const;
    bool exists(std::string_view namespace_s, std::string_view path) const;

    // winning file for every path below `directory` (a directory of a namespace, e.g. "loot_table") having the extension.
    // resolved_file_t::id() of such a file is still relative to the namespace directory: use list_layers for registry relative ids
    std::vector<resolved_file_t> list(std::string_view directory, std::string_view extension) const;
    // the file with the given namespace + path, nullopt when absent
    std::optional<resolved_file_t> find(std::string_view namespace_s, std::string_view path) const;
    // every visible layer of the file, lowest pack first (used for tags)
    std::vector<resolved_file_t> layers(std::string_view namespace_s, std::string_view path) const;
    // all visible layers of every file below `directory` having the extension; the id is relative to `directory`
    // (data/minecraft/loot_table/blocks/stone.json -> minecraft:blocks/stone)
    std::vector<std::pair<resource_location_t, std::vector<resolved_file_t>>> list_layers(std::string_view directory, std::string_view extension) const;
};

}
