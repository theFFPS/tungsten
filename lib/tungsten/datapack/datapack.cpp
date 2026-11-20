/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/datapack/datapack.hpp>

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>

namespace tungsten {

namespace {

// "<prefix><namespace>/<path>" -> { namespace, path }
bool split_data_path(std::string_view full, std::string_view prefix, std::string& namespace_s, std::string& path) {
    if (!full.starts_with(prefix)) return false;
    full.remove_prefix(prefix.size());
    auto slash = full.find('/');
    if (slash == std::string_view::npos || slash == 0 || slash + 1 >= full.size()) return false;
    namespace_s = full.substr(0, slash);
    path = full.substr(slash + 1);
    return true;
}

}

std::shared_ptr<datapack_t> datapack_t::open(const std::filesystem::path& path) {
    auto name = path.filename().string();
    if (name.empty()) name = path.parent_path().filename().string();
    return open(open_pack_source(path), name);
}
std::shared_ptr<datapack_t> datapack_t::open(std::shared_ptr<pack_source_t> source, std::string name) {
    if (!source->exists("pack.mcmeta")) {
        throw std::runtime_error("datapack_t::open error: '" + source->describe() + "' has no pack.mcmeta");
    }
    auto bytes = source->read("pack.mcmeta");
    std::shared_ptr<datapack_t> pack (new datapack_t());
    try {
        pack->m_meta = pack_meta_t::parse(std::string_view(reinterpret_cast<const char*>(bytes.data()), bytes.size()));
    } catch (const std::exception& e) {
        throw std::runtime_error("datapack_t::open error: " + source->describe() + ": " + e.what());
    }
    pack->m_name = std::move(name);
    pack->m_source = std::move(source);
    return pack;
}
std::shared_ptr<datapack_t> datapack_t::open_data_directory(const std::filesystem::path& path, std::string name) {
    std::shared_ptr<datapack_t> pack (new datapack_t());
    pack->m_source = std::make_shared<directory_pack_source_t>(path);
    pack->m_name = std::move(name);
    pack->m_bare_data_directory = true;
    // a synthetic meta which supports the current format
    pack->m_meta.m_description = pack->m_name;
    pack->m_meta.m_range = { current_data_pack_version, { current_data_pack_version.m_major, pack_version_t::any_minor } };
    return pack;
}

datapack_t::compatibility_t datapack_t::compatibility(const pack_version_t& version) const noexcept {
    if (version < m_meta.m_range.m_min) return compatibility_t::too_new; // the pack needs a newer game
    if (version > m_meta.m_range.m_max) return compatibility_t::too_old; // the pack was made for an older game
    return compatibility_t::compatible;
}

std::vector<datapack_file_t> datapack_t::files(const pack_version_t& version) const {
    std::map<std::pair<std::string, std::string>, datapack_file_t> result;
    auto collect = [&](std::string_view prefix, const std::string& overlay) {
        std::string namespace_s, path;
        for (const auto& full : m_source->list_files()) {
            if (!split_data_path(full, prefix, namespace_s, path)) continue;
            result[{ namespace_s, path }] = datapack_file_t { namespace_s, path, full, overlay };
        }
    };
    if (m_bare_data_directory) {
        collect("", "");
    } else {
        collect("data/", "");
        for (const auto& overlay : m_meta.active_overlays(version)) collect(overlay + "/data/", overlay);
    }
    std::vector<datapack_file_t> files;
    files.reserve(result.size());
    for (auto& [key, file] : result) files.push_back(std::move(file));
    return files;
}
std::vector<std::uint8_t> datapack_t::read(const datapack_file_t& file) const {
    return m_source->read(file.m_source_path);
}

std::vector<std::shared_ptr<datapack_t>> datapack_t::bundled_packs() const {
    std::vector<std::shared_ptr<datapack_t>> result;
    std::set<std::string> pack_roots;
    std::string namespace_s, path;
    std::string prefix = m_bare_data_directory ? "" : "data/";
    for (const auto& full : m_source->list_files()) {
        if (!split_data_path(full, prefix, namespace_s, path) || !path.starts_with("datapacks/")) continue;
        auto rest = std::string_view(path).substr(10);
        auto slash = rest.find('/');
        if (slash == std::string_view::npos) continue;
        pack_roots.insert(prefix + namespace_s + "/datapacks/" + std::string(rest.substr(0, slash)));
    }
    for (const auto& root : pack_roots) {
        // a sub source which exposes only the files below `root`
        struct sub_source_t final : public pack_source_t {
            std::shared_ptr<pack_source_t> m_parent;
            std::string m_root;
            std::vector<std::string> m_files;
            sub_source_t(std::shared_ptr<pack_source_t> parent, std::string root) : m_parent(std::move(parent)), m_root(std::move(root)) {
                for (const auto& f : m_parent->list_files()) {
                    if (f.size() > m_root.size() + 1 && f.starts_with(m_root) && f[m_root.size()] == '/') m_files.push_back(f.substr(m_root.size() + 1));
                }
            }
            std::string describe() const override { return m_parent->describe() + "/" + m_root; }
            const std::vector<std::string>& list_files() const override { return m_files; }
            bool exists(std::string_view p) const override { return m_parent->exists(m_root + "/" + std::string(p)); }
            std::vector<std::uint8_t> read(std::string_view p) const override { return m_parent->read(m_root + "/" + std::string(p)); }
        };
        auto source = std::make_shared<sub_source_t>(m_source, root);
        if (!source->exists("pack.mcmeta")) continue;
        auto pack = open(source, root.substr(root.rfind('/') + 1));
        pack->m_bundled = true;
        result.push_back(std::move(pack));
    }
    return result;
}

}
