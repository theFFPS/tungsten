/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/datapack/datapack_stack.hpp>

#include <algorithm>

namespace tungsten {

resource_location_t resolved_file_t::id(std::string_view extension) const {
    std::string_view path = m_file.m_path;
    if (path.ends_with(extension)) path.remove_suffix(extension.size());
    return resource_location_t(m_file.m_namespace, path);
}

void datapack_stack_t::add(std::shared_ptr<datapack_t> pack) {
    m_packs.push_back(std::move(pack));
    m_index_valid = false;
}
void datapack_stack_t::clear() {
    m_packs.clear();
    m_index_valid = false;
}
bool datapack_stack_t::remove(std::string_view name) {
    auto it = std::find_if(m_packs.begin(), m_packs.end(), [name](const auto& p) { return p->name() == name; });
    if (it == m_packs.end()) return false;
    m_packs.erase(it);
    m_index_valid = false;
    return true;
}

void datapack_stack_t::build_index() const {
    m_index.clear();
    for (std::size_t i = 0; i < m_packs.size(); ++i) {
        for (auto& file : m_packs[i]->files(m_version)) {
            // hidden by the filter of any pack above this one?
            bool hidden = false;
            for (std::size_t k = i + 1; k < m_packs.size() && !hidden; ++k) {
                hidden = m_packs[k]->meta().is_filtered(file.m_namespace, file.m_path);
            }
            if (hidden) continue;
            auto& layers = m_index[file.m_namespace][file.m_path];
            layers.push_back({ i, std::move(file) });
        }
    }
    m_index_valid = true;
}
const std::map<std::string, datapack_stack_t::path_index_t>& datapack_stack_t::index() const {
    if (!m_index_valid) build_index();
    return m_index;
}

std::set<std::string> datapack_stack_t::namespaces() const {
    std::set<std::string> result;
    for (const auto& [name, files] : index()) result.insert(name);
    return result;
}
bool datapack_stack_t::exists(std::string_view namespace_s, std::string_view path) const {
    return find(namespace_s, path).has_value();
}

std::vector<resolved_file_t> datapack_stack_t::list(std::string_view directory, std::string_view extension) const {
    std::vector<resolved_file_t> result;
    for (auto& [id, layers] : list_layers(directory, extension)) result.push_back(std::move(layers.back()));
    return result;
}
std::optional<resolved_file_t> datapack_stack_t::find(std::string_view namespace_s, std::string_view path) const {
    auto layers_of_file = layers(namespace_s, path);
    if (layers_of_file.empty()) return std::nullopt;
    return std::move(layers_of_file.back());
}
std::vector<resolved_file_t> datapack_stack_t::layers(std::string_view namespace_s, std::string_view path) const {
    std::vector<resolved_file_t> result;
    const auto& idx = index();
    auto ns = idx.find(std::string(namespace_s));
    if (ns == idx.end()) return result;
    auto file = ns->second.find(std::string(path));
    if (file == ns->second.end()) return result;
    for (const auto& layer : file->second) result.push_back({ m_packs[layer.m_pack_index], layer.m_file });
    return result;
}
std::vector<std::pair<resource_location_t, std::vector<resolved_file_t>>> datapack_stack_t::list_layers(
    std::string_view directory, std::string_view extension) const {
    std::vector<std::pair<resource_location_t, std::vector<resolved_file_t>>> result;
    std::string prefix = std::string(directory) + "/";
    for (const auto& [namespace_s, files] : index()) {
        for (auto it = files.lower_bound(prefix); it != files.end() && it->first.starts_with(prefix); ++it) {
            const std::string& path = it->first;
            if (!path.ends_with(extension) || path.size() == prefix.size() + extension.size()) continue;
            std::vector<resolved_file_t> layers;
            for (const auto& layer : it->second) layers.push_back({ m_packs[layer.m_pack_index], layer.m_file });
            // the id is relative to `directory`: data/minecraft/loot_table/blocks/stone.json -> minecraft:blocks/stone
            std::string_view id_path = std::string_view(path).substr(prefix.size(), path.size() - prefix.size() - extension.size());
            result.emplace_back(resource_location_t(namespace_s, id_path), std::move(layers));
        }
    }
    return result;
}

}
