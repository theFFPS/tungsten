/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <tungsten/util/json_reflect.hpp>
#include <tungsten/datapack/datapack_stack.hpp>
#include <tungsten/datapack/load_report.hpp>
#include <tungsten/resource/resource_location.hpp>

namespace tungsten {

// one entry of a tag: an element id, "#tag", optionally not required to exist
struct tag_entry_t {
    bool m_is_tag = false;
    bool m_required = true;
    resource_location_t m_id;

    bool operator==(const tag_entry_t&) const = default;
    // "id", "#tag" or { "id": "id" | "#tag", "required": false }
    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = tag_entry_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

// contents of one tag file: { "replace": false, "values": [ "minecraft:stone", "#minecraft:base_stone_overworld", { "id": "x:y", "required": false } ] }
struct tag_file_t {
    bool m_replace = false;
    std::vector<tag_entry_t> m_values;

    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = tag_file_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

// the merged (but not yet flattened) definition of a tag
struct tag_t {
    std::vector<tag_entry_t> m_entries;
    std::vector<std::string> m_packs; // packs which contributed, lowest first
};

// all tags of one registry, e.g. "block" or "worldgen/biome"
struct tag_registry_t {
    resource_location_t m_registry;
    std::map<resource_location_t, tag_t> m_tags;
    // flattened tag contents (no nested tags, duplicates removed, definition order kept)
    std::map<resource_location_t, std::vector<resource_location_t>> m_flattened;

    bool has_tag(const resource_location_t& id) const { return m_tags.contains(id); }
    // throws std::runtime_error for an unknown tag
    const std::vector<resource_location_t>& elements(const resource_location_t& tag) const;
    bool contains(const resource_location_t& tag, const resource_location_t& element) const;
};

// Loads and merges tags of every registry. Merge rules (same as the game):
//  - tag files of the same id are merged in pack order; "replace": true discards everything gathered from lower packs
//  - '#' entries are resolved recursively; cycles and missing required entries are reported
struct tag_manager_t {
private:
    std::map<std::string, tag_registry_t> m_registries; // key: registry path ("worldgen/biome")
public:
    // registries whose tags are known. A path below tags/ belongs to the longest matching registry.
    static const std::vector<std::string>& default_registry_names();

    // `element_exists` (optional) lets the loader validate plain entries, registry -> id -> exists
    using exists_fn_t = std::function<bool(std::string_view registry, const resource_location_t& id)>;

    void load(const datapack_stack_t& stack, load_report_t& report, const std::vector<std::string>& registry_names = default_registry_names());
    // validates entries with `exists` and flattens; call after all registries have been loaded
    void flatten(load_report_t& report, const exists_fn_t& exists = nullptr);

    bool has_registry(std::string_view registry) const { return m_registries.contains(std::string(registry)); }
    const tag_registry_t* registry(std::string_view registry) const;
    const tag_registry_t& require_registry(std::string_view registry) const;
    const std::map<std::string, tag_registry_t>& registries() const noexcept { return m_registries; }
    std::size_t tag_count() const noexcept;
};

}
