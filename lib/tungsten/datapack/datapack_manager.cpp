/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/datapack/datapack_manager.hpp>

#include <algorithm>
#include <future>
#include <sstream>

namespace tungsten {

void datapack_manager_t::add_base_directory(const std::filesystem::path& path) {
    if (std::filesystem::exists(path / "pack.mcmeta") || !std::filesystem::is_directory(path)) m_base = datapack_t::open(path);
    else m_base = datapack_t::open_data_directory(path, "vanilla");
    m_base->set_bundled(false);
    m_bundled = m_base->bundled_packs();
    m_loaded = false;
    rebuild_stack();
}
std::shared_ptr<datapack_t> datapack_manager_t::add_pack(const std::filesystem::path& path) {
    auto pack = datapack_t::open(path);
    add_pack(pack);
    return pack;
}
void datapack_manager_t::add_pack(std::shared_ptr<datapack_t> pack) {
    switch (pack->compatibility(m_stack.version())) {
    case datapack_t::compatibility_t::too_old:
        m_report.warning("", "", pack->name(), "pack format " + pack->meta().m_range.m_max.to_string() + " is older than the game's "
            + m_stack.version().to_string() + ": the pack may not work correctly");
        break;
    case datapack_t::compatibility_t::too_new:
        m_report.warning("", "", pack->name(), "pack format " + pack->meta().m_range.m_min.to_string() + " is newer than the game's "
            + m_stack.version().to_string() + ": the pack may not work correctly");
        break;
    default: break;
    }
    m_user_packs.push_back(std::move(pack));
    m_loaded = false;
    rebuild_stack();
}
void datapack_manager_t::add_packs_from_directory(const std::filesystem::path& path) {
    std::vector<std::filesystem::path> entries;
    std::error_code ec;
    for (std::filesystem::directory_iterator it (path, ec), end; !ec && it != end; it.increment(ec)) {
        const auto& name = it->path().filename().string();
        if (name.starts_with('.')) continue;
        if (it->is_directory(ec) || (it->is_regular_file(ec) && it->path().extension() == ".zip")) entries.push_back(it->path());
    }
    std::sort(entries.begin(), entries.end());
    for (const auto& entry : entries) {
        try {
            add_pack(entry);
        } catch (const std::exception& e) {
            m_report.error("", "", entry.filename().string(), e.what());
        }
    }
}
bool datapack_manager_t::remove_pack(std::string_view name) {
    auto it = std::find_if(m_user_packs.begin(), m_user_packs.end(), [name](const auto& p) { return p->name() == name; });
    if (it == m_user_packs.end()) return false;
    m_user_packs.erase(it);
    m_loaded = false;
    rebuild_stack();
    return true;
}
void datapack_manager_t::enable_feature(const resource_location_t& feature) {
    m_enabled_features.insert(feature);
    m_loaded = false;
    rebuild_stack();
}
void datapack_manager_t::disable_feature(const resource_location_t& feature) {
    m_enabled_features.erase(feature);
    m_loaded = false;
    rebuild_stack();
}

void datapack_manager_t::rebuild_stack() {
    m_stack.clear();
    if (m_base) m_stack.add(m_base);
    // a bundled feature pack is active when one of the features it enables is enabled; it sits above the base data, below user packs
    for (const auto& bundled : m_bundled) {
        const auto& features = bundled->meta().m_enabled_features;
        bool active = std::any_of(features.begin(), features.end(), [this](const auto& f) { return m_enabled_features.contains(f); });
        if (active) m_stack.add(bundled);
    }
    for (const auto& pack : m_user_packs) m_stack.add(pack);
}

void datapack_manager_t::load() {
    // keep warnings produced by add_pack(), drop everything from a previous load
    registry::clear_all();
    m_stack.prepare();

    std::vector<std::future<void>> jobs;
    for (auto* registry : registry::all()) {
        jobs.push_back(std::async(std::launch::async, [this, registry] {
            try {
                registry->load(m_stack, m_report);
            } catch (const std::exception& e) {
                m_report.error(registry->identifier().m_path, "", "", e.what());
            }
        }));
    }
    for (auto& job : jobs) job.get();

    m_tags.load(m_stack, m_report);
    // tags may only reference elements of registries which are loaded from data packs; other registries (block, item...) are built in
    m_tags.flatten(m_report, [](std::string_view registry_name, const resource_location_t& id) {
        auto* registry = registry::by_name(registry_name);
        return registry == nullptr || registry->contains(id);
    });
    m_loaded = true;
}

std::string datapack_manager_t::summary() const {
    std::ostringstream out;
    for (auto* registry : registry::all()) {
        if (registry->size() != 0) out << registry->identifier().m_path << ": " << registry->size() << "\n";
    }
    out << "tags: " << m_tags.tag_count() << "\n";
    return out.str();
}

}
