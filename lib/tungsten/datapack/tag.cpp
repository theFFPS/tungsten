/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/resource/tag.hpp>

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

void tag_entry_t::from_generic(const glz::generic& j) {
    *this = tag_entry_t();
    std::string text;
    if (j.is_string()) {
        text = j.get_string();
    } else if (j.is_object()) {
        if (!j.contains("id") || !j["id"].is_string()) throw jr::error("", "tag entry object requires a string 'id'");
        text = j["id"].get_string();
        if (j.contains("required")) m_required = jr::from_generic<bool>(j["required"]);
    } else {
        throw jr::error("", "tag entry must be a string or an object");
    }
    if (text.starts_with('#')) {
        m_is_tag = true;
        text.erase(0, 1);
    }
    m_id = resource_location_t(text);
}
glz::generic tag_entry_t::to_generic() const {
    std::string text = m_is_tag ? "#" + m_id.to_string() : m_id.to_string();
    if (m_required) return glz::generic(text);
    glz::generic j = glz::generic::object_t{};
    j["id"] = text;
    j["required"] = false;
    return j;
}
void tag_file_t::from_generic(const glz::generic& j) {
    *this = tag_file_t();
    if (!j.is_object()) throw jr::error("", "expected an object");
    if (j.contains("replace")) m_replace = jr::from_generic<bool>(j["replace"]);
    if (!j.contains("values") || !j["values"].is_array()) throw jr::error("", "missing required array 'values'");
    std::size_t index = 0;
    for (const auto& entry : j["values"].get_array()) {
        try {
            tag_entry_t value;
            value.from_generic(entry);
            m_values.push_back(std::move(value));
        } catch (const std::exception& e) {
            throw jr::error("values[" + std::to_string(index) + "]", e.what());
        }
        ++index;
    }
}
glz::generic tag_file_t::to_generic() const {
    glz::generic j = glz::generic::object_t{};
    glz::generic::array_t values;
    for (const auto& value : m_values) values.push_back(value.to_generic());
    j["values"] = glz::generic(std::move(values));
    if (m_replace) j["replace"] = true;
    return j;
}

const std::vector<resource_location_t>& tag_registry_t::elements(const resource_location_t& tag) const {
    auto it = m_flattened.find(tag);
    if (it == m_flattened.end()) {
        throw std::runtime_error("tag_registry_t::elements error: unknown tag #" + tag.to_string() + " in " + m_registry.to_string());
    }
    return it->second;
}
bool tag_registry_t::contains(const resource_location_t& tag, const resource_location_t& element) const {
    auto it = m_flattened.find(tag);
    if (it == m_flattened.end()) return false;
    return std::find(it->second.begin(), it->second.end(), element) != it->second.end();
}

const std::vector<std::string>& tag_manager_t::default_registry_names() {
    static const std::vector<std::string> names = {
        "banner_pattern", "block", "cat_variant", "chicken_variant", "cow_variant", "damage_type", "dialog", "enchantment", "entity_type",
        "fluid", "frog_variant", "function", "game_event", "instrument", "item", "jukebox_song", "painting_variant", "pig_variant",
        "point_of_interest_type", "potion", "timeline", "trim_material", "trim_pattern", "villager_trade", "wolf_variant",
        "worldgen/biome", "worldgen/flat_level_generator_preset", "worldgen/structure", "worldgen/world_preset", "worldgen/feature",
        "worldgen/placed_feature", "worldgen/template_pool", "worldgen/structure_set", "worldgen/processor_list", "worldgen/noise",
        "worldgen/density_function", "worldgen/noise_settings", "worldgen/carver", "worldgen/multi_noise_biome_source_parameter_list",
        "wolf_sound_variant", "zombie_nautilus_variant", "loot_table", "recipe", "advancement", "world_clock", "trade_set",
        "test_environment", "test_instance", "sulfur_cube_archetype"
    };
    return names;
}

void tag_manager_t::load(const datapack_stack_t& stack, load_report_t& report, const std::vector<std::string>& registry_names) {
    m_registries.clear();
    // longest registry names first so that "worldgen/biome" wins over a hypothetical "worldgen"
    std::vector<std::string> names = registry_names;
    std::sort(names.begin(), names.end(), [](const auto& a, const auto& b) { return a.size() > b.size(); });

    for (const auto& [id, layers] : stack.list_layers("tags", ".json")) {
        // id.m_path = "<registry...>/<tag path>" below tags/
        std::string_view path = id.m_path; // relative to tags/
        const std::string* registry_name = nullptr;
        for (const auto& name : names) {
            if (path.size() > name.size() + 1 && path.starts_with(name) && path[name.size()] == '/') {
                registry_name = &name;
                break;
            }
        }
        if (registry_name == nullptr) {
            report.warning("tags", id.to_string(), layers.back().m_pack->name(), "tag of an unknown registry ignored");
            continue;
        }
        resource_location_t tag_id (id.m_namespace, path.substr(registry_name->size() + 1));
        auto& registry = m_registries[*registry_name];
        registry.m_registry = resource_location_t(*registry_name);
        tag_t merged;
        for (const auto& layer : layers) {
            try {
                auto bytes = layer.m_pack->read(layer.m_file);
                auto file = jr::read<tag_file_t>(std::string_view(reinterpret_cast<const char*>(bytes.data()), bytes.size()));
                if (file.m_replace) merged.m_entries.clear();
                for (auto& entry : file.m_values) {
                    // avoid duplicates but keep first occurrence order
                    if (std::find(merged.m_entries.begin(), merged.m_entries.end(), entry) == merged.m_entries.end()) {
                        merged.m_entries.push_back(std::move(entry));
                    }
                }
                merged.m_packs.push_back(layer.m_pack->name());
            } catch (const std::exception& e) {
                report.error("tags/" + *registry_name, tag_id.to_string(), layer.m_pack->name(), e.what());
            }
        }
        registry.m_tags[tag_id] = std::move(merged);
    }
}

void tag_manager_t::flatten(load_report_t& report, const exists_fn_t& exists) {
    for (auto& [registry_name, registry] : m_registries) {
        registry.m_flattened.clear();
        std::set<resource_location_t> in_progress;
        std::function<bool(const resource_location_t&)> resolve = [&](const resource_location_t& tag_id) -> bool {
            if (registry.m_flattened.contains(tag_id)) return true;
            auto it = registry.m_tags.find(tag_id);
            if (it == registry.m_tags.end()) return false;
            if (!in_progress.insert(tag_id).second) {
                report.error("tags/" + registry_name, tag_id.to_string(), "", "tag reference cycle detected");
                return false;
            }
            std::vector<resource_location_t> result;
            auto add_unique = [&result](const resource_location_t& id) {
                if (std::find(result.begin(), result.end(), id) == result.end()) result.push_back(id);
            };
            for (const auto& entry : it->second.m_entries) {
                if (entry.m_is_tag) {
                    if (resolve(entry.m_id)) {
                        for (const auto& id : registry.m_flattened[entry.m_id]) add_unique(id);
                    } else if (entry.m_required && !in_progress.contains(entry.m_id)) {
                        report.error("tags/" + registry_name, tag_id.to_string(), it->second.m_packs.empty() ? "" : it->second.m_packs.back(),
                            "references missing tag #" + entry.m_id.to_string());
                    }
                } else {
                    if (entry.m_required && exists && !exists(registry_name, entry.m_id)) {
                        report.error("tags/" + registry_name, tag_id.to_string(), it->second.m_packs.empty() ? "" : it->second.m_packs.back(),
                            "references missing element " + entry.m_id.to_string());
                    }
                    add_unique(entry.m_id);
                }
            }
            in_progress.erase(tag_id);
            registry.m_flattened[tag_id] = std::move(result);
            return true;
        };
        for (const auto& [tag_id, tag] : registry.m_tags) resolve(tag_id);
    }
}

const tag_registry_t* tag_manager_t::registry(std::string_view name) const {
    auto it = m_registries.find(std::string(name));
    return it == m_registries.end() ? nullptr : &it->second;
}
const tag_registry_t& tag_manager_t::require_registry(std::string_view name) const {
    const auto* result = registry(name);
    if (result == nullptr) throw std::runtime_error("tag_manager_t::require_registry error: no tags loaded for registry " + std::string(name));
    return *result;
}
std::size_t tag_manager_t::tag_count() const noexcept {
    std::size_t count = 0;
    for (const auto& [name, registry] : m_registries) count += registry.m_tags.size();
    return count;
}

}
