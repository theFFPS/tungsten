/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/util/json_range.hpp>
#include <tungsten/util/json_reflect.hpp>
#include <tungsten/resource/holder_set.hpp>
#include <tungsten/world/attribute/environment_attribute.hpp>

namespace tungsten {

// keys are resource locations ("temperature" or "minecraft:temperature"), values are ranges (offset is a plain number)
struct biome_noise_parameters_t {
    std::optional<float_range_t> temperature;
    std::optional<float_range_t> humidity;
    std::optional<float_range_t> continentalness;
    std::optional<float_range_t> erosion;
    std::optional<float_range_t> weirdness;
    std::optional<float_range_t> depth;
    std::optional<float> offset;

    void from_generic(const glz::generic& j) {
        if (!j.is_object()) throw jr::error("", "expected an object");
        *this = {};
        for (const auto& [key, val] : j.get_object()) {
            const resource_location_t rl_key(key);
            auto range = [&] { float_range_t r; r.from_generic(val); return r; };
            if (rl_key == resource_location_t("temperature")) temperature = range();
            else if (rl_key == resource_location_t("humidity")) humidity = range();
            else if (rl_key == resource_location_t("continentalness")) continentalness = range();
            else if (rl_key == resource_location_t("erosion")) erosion = range();
            else if (rl_key == resource_location_t("weirdness")) weirdness = range();
            else if (rl_key == resource_location_t("depth")) depth = range();
            else if (rl_key == resource_location_t("offset")) offset = static_cast<float>(val.get_number());
        }
    }
    glz::generic to_generic() const {
        glz::generic j = glz::generic::object_t{};
        auto put = [&](const char* key, const std::optional<float_range_t>& r) { if (r.has_value()) j[key] = r->to_generic(); };
        put("temperature", temperature);
        put("humidity", humidity);
        put("continentalness", continentalness);
        put("erosion", erosion);
        put("weirdness", weirdness);
        put("depth", depth);
        if (offset.has_value()) j["offset"] = static_cast<double>(*offset);
        return j;
    }

    struct glaze {
        using R = biome_noise_parameters_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};
struct biome_t {
    struct effects_t {
        std::string water_color;
        std::optional<std::string> foliage_color;
        std::optional<std::string> dry_foliage_color;
        std::optional<std::string> grass_color;
        std::optional<std::string> grass_color_modifier; // none || dark_forest || swamp
    };

    bool has_precipitation;
    float temperature;
    std::optional<std::string> temperature_modifier; // none || frozen
    float downfall;
    effects_t effects;
    // environment attributes ("minecraft:visual/sky_color" ...), values depend on the attribute
    environment_attribute_map_t attributes = {};
    // a tag / id / list of ids
    std::variant<std::vector<resource_location_t>, resource_or_tag_t> carvers = std::vector<resource_location_t> {};
    // one list per generation step (11 steps); entries are placed feature ids / tags
    std::vector<std::variant<std::vector<resource_or_tag_t>, resource_or_tag_t>> features = {};
};

}
