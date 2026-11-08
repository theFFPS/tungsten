/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <stdexcept>
#include <tungsten/type/variant.hpp>
#include <tungsten/resource/resource_location.hpp>

namespace tungsten {

struct flat_generation_settings_t {
    struct layer_t {
        int m_height;
        resource_location_t m_block = resource_location_t("air");

        friend void to_json(nlohmann::json& j, const flat_generation_settings_t::layer_t& val) {
            j = {
                { "height", val.m_height },
                { "block", val.m_block }
            };
        }
        friend void from_json(const nlohmann::json& j, flat_generation_settings_t::layer_t& val) {
            val.m_height = j.at("height");
            if (j.contains("block")) val.m_block = j["block"];
            else val.m_block = resource_location_t("air");
        }
    };

    std::vector<layer_t> m_layers;
    resource_location_t m_biome = resource_location_t("plains");
    bool m_lakes = false;
    bool m_features = false;
    std::variant<std::string, std::vector<resource_location_t>> m_structure_overrides;

    friend void to_json(nlohmann::json& j, const flat_generation_settings_t& val) {
        nlohmann::json structure_overrides_json;
        std::visit(overloaded{
            [&structure_overrides_json](const std::string& value) {
                structure_overrides_json = value;
            },
            [&structure_overrides_json](const std::vector<resource_location_t>& value) {
                structure_overrides_json = value;
            }
        }, val.m_structure_overrides);

        j = {
            { "layers", val.m_layers },
            { "biome", val.m_biome },
            { "lakes", val.m_lakes },
            { "features", val.m_features },
            { "structure_overrides", structure_overrides_json }
        };
    }
    friend void from_json(const nlohmann::json& j, flat_generation_settings_t& val) {
        val.m_layers = j["layers"];
        val.m_lakes = (j.contains("lakes") ? (bool) j["lakes"] : false);
        val.m_features = (j.contains("features") ? (bool) j["features"] : false);
        val.m_biome = (j.contains("biome") ? (resource_location_t) j["biome"] : resource_location_t("plains"));
        if (j.contains("structure_overrides")) {
            if (j["structure_overrides"].is_string()) {
                val.m_structure_overrides = std::string(j["structure_overrides"]);
            } else if (j["structure_overrides"].is_array()) {
                val.m_structure_overrides = std::vector<resource_location_t>(j["structure_overrides"]);
            } else {
                throw std::runtime_error("flat_generation_settings_t::from_json error: unexpected type of 'structure_overrides'");
            }
        }
    }
};

struct flat_level_generator_preset_t {
    resource_location_t display;
    flat_generation_settings_t settings;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(flat_level_generator_preset_t, display, settings)

}
