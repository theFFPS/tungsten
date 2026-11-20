/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <tungsten/datapack/pack_meta.hpp>
#include <tungsten/datapack/pack_source.hpp>
#include <tungsten/resource/resource_location.hpp>

namespace tungsten {

// a file inside of `data/<namespace>/` of a data pack
struct datapack_file_t {
    std::string m_namespace;
    std::string m_path;        // relative to the namespace directory, with extension: "loot_table/blocks/stone.json"
    std::string m_source_path; // path inside of the pack source (includes the overlay directory when applicable)
    std::string m_overlay;     // empty for the base pack
};

struct datapack_t {
    enum class compatibility_t { compatible, too_old, too_new };

private:
    std::string m_name;
    std::shared_ptr<pack_source_t> m_source;
    pack_meta_t m_meta;
    bool m_bare_data_directory = false; // namespaces are located in the pack root, there is no data/ directory nor pack.mcmeta
    bool m_bundled = false;

    datapack_t() = default;
public:
    // directory or .zip containing pack.mcmeta. throws std::runtime_error when pack.mcmeta is missing or malformed
    static std::shared_ptr<datapack_t> open(const std::filesystem::path& path);
    static std::shared_ptr<datapack_t> open(std::shared_ptr<pack_source_t> source, std::string name);
    // a directory which holds namespaces directly (e.g. the extracted "data" directory of the game jar)
    static std::shared_ptr<datapack_t> open_data_directory(const std::filesystem::path& path, std::string name);

    const std::string& name() const noexcept { return m_name; }
    const pack_meta_t& meta() const noexcept { return m_meta; }
    const std::shared_ptr<pack_source_t>& source() const noexcept { return m_source; }
    bool is_bundled() const noexcept { return m_bundled; }
    void set_bundled(bool value) noexcept { m_bundled = value; }

    compatibility_t compatibility(const pack_version_t& version = current_data_pack_version) const noexcept;

    // all data files with overlays applied (a file of a later overlay replaces the same file of the base pack / earlier overlays).
    // pack.mcmeta filters are NOT applied here: they belong to the packs loaded after this one (see datapack_stack_t)
    std::vector<datapack_file_t> files(const pack_version_t& version = current_data_pack_version) const;
    std::vector<std::uint8_t> read(const datapack_file_t& file) const;
    // bundled feature packs found in <namespace>/datapacks/<name> (like the vanilla jar does)
    std::vector<std::shared_ptr<datapack_t>> bundled_packs() const;
};

}
