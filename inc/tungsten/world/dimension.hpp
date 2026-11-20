/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/util/json_reflect.hpp>
#include <tungsten/resource/resource_location.hpp>
#include <tungsten/resource/holder_set.hpp>
#include <tungsten/type/int_provider.hpp>
#include "worldgen/flat_level_generator_preset.hpp"
#include "worldgen/biome.hpp"
#include "worldgen/noise.hpp"
#include "attribute/environment_attribute.hpp"

namespace tungsten {

struct dimension_type_t {
    double coordinate_scale;
    bool has_ceiling;
    bool has_ender_dragon_fight;
    bool has_skylight;
    float ambient_light;
    bool has_fixed_time = false;
    int monster_spawn_block_light_limit;
    int_provider_t monster_spawn_light_level;
    int logical_height;
    int min_y;
    int height;
    holder_set_t infiniburn;
    std::string skybox = "overworld"; // overworld || end || none
    std::string cardinal_light = "default"; // default || nether
    environment_attribute_map_t attributes;
    std::optional<resource_location_t> default_clock;
    holder_set_t timelines;
};

struct dimension_t {
    struct generator__debug_t {
        static constexpr std::string_view type_tag = "debug";
    };
    struct generator__flat_t {
        static constexpr std::string_view type_tag = "flat";
        flat_generation_settings_t settings;
    };
    struct generator__noise_t {
        struct biome_source__checkerboard_t {
            static constexpr std::string_view type_tag = "checkerboard";
            holder_set_t biomes;
            int scale = 2;
        };
        struct biome_source__fixed_t {
            static constexpr std::string_view type_tag = "fixed";
            resource_location_t biome;
        };
        struct biome_source__multi_noise_t {
            struct biome_parameter_t {
                resource_location_t biome;
                biome_noise_parameters_t parameters;
            };

            static constexpr std::string_view type_tag = "multi_noise";
            std::optional<resource_location_t> preset; // overworld || nether
            std::optional<std::vector<biome_parameter_t>> biomes;
        };
        struct biome_source__the_end_t {
            static constexpr std::string_view type_tag = "the_end";
        };
        using biome_source_t = std::variant<biome_source__multi_noise_t, biome_source__checkerboard_t, biome_source__fixed_t, biome_source__the_end_t>;

        static constexpr std::string_view type_tag = "noise";
        holder_t<noise_settings_t> settings;
        std::variant<biome_source_t, resource_location_t> biome_source;
    };
    using generator_t = std::variant<generator__debug_t, generator__flat_t, generator__noise_t>;

    resource_location_t type;
    generator_t generator;
};

}
