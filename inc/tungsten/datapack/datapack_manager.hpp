/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <filesystem>
#include <set>
#include <string>
#include <vector>
#include <tungsten/datapack/datapack_stack.hpp>
#include <tungsten/datapack/load_report.hpp>
#include <tungsten/resource/builtin_registries.hpp>
#include <tungsten/resource/tag.hpp>

namespace tungsten {

// Loads data packs into the global registries (tungsten::registry::*) and the tag manager.
//
//   datapack_manager_t manager;
//   manager.add_base_directory("test/data");                      // vanilla data: <dir>/<namespace>/<registry>/...
//   manager.add_packs_from_directory("saves/world/datapacks");    // alphabetical = load order, later packs override earlier ones
//   manager.enable_feature("minecraft:trade_rebalance");          // bundled experimental packs are only used when enabled
//   manager.load();
//   if (!manager.report().ok()) { ... }
struct datapack_manager_t {
private:
    datapack_stack_t m_stack;
    load_report_t m_report;
    tag_manager_t m_tags;
    std::set<resource_location_t> m_enabled_features;
    std::vector<std::shared_ptr<datapack_t>> m_bundled; // found in the base data, not in the stack until their feature is enabled
    bool m_loaded = false;

    void rebuild_stack();
    std::vector<std::shared_ptr<datapack_t>> m_user_packs;
    std::shared_ptr<datapack_t> m_base;
public:
    datapack_manager_t() = default;
    explicit datapack_manager_t(pack_version_t version) : m_stack(version) {}

    // the lowest pack: the vanilla data (a directory which holds namespaces directly, or a pack with pack.mcmeta)
    void add_base_directory(const std::filesystem::path& path);
    // adds a pack (directory / zip with pack.mcmeta) on top
    std::shared_ptr<datapack_t> add_pack(const std::filesystem::path& path);
    // adds every pack directory / .zip of a directory, in alphabetical order. entries without pack.mcmeta are reported and skipped
    void add_packs_from_directory(const std::filesystem::path& path);
    void add_pack(std::shared_ptr<datapack_t> pack);
    bool remove_pack(std::string_view name);

    void enable_feature(const resource_location_t& feature);
    void disable_feature(const resource_location_t& feature);
    const std::set<resource_location_t>& enabled_features() const noexcept { return m_enabled_features; }
    // bundled packs discovered in the base data (their names are the directory names, e.g. "trade_rebalance")
    const std::vector<std::shared_ptr<datapack_t>>& bundled_packs() const noexcept { return m_bundled; }

    // (re)loads every registry and tag from the current packs; results are available in registry::* and tags()
    void load();
    bool loaded() const noexcept { return m_loaded; }

    const datapack_stack_t& stack() const noexcept { return m_stack; }
    const load_report_t& report() const noexcept { return m_report; }
    const tag_manager_t& tags() const noexcept { return m_tags; }
    // one line per registry: "<directory>: <count>"
    std::string summary() const;
};

}
