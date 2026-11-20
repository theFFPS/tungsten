/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/util/json_reflect.hpp>
#include <tungsten/type/int_provider.hpp>
#include <tungsten/resource/holder_set.hpp>
#include "template_pool.hpp"

namespace tungsten {

struct structure_t {
    struct spawn_override_t {
        struct structure_spawn_data_t {
            resource_location_t type;
            float weight;
            int_provider_t count;
        };

        std::string bounding_box; // piece || full
        std::vector<structure_spawn_data_t> spawns;
    };
    struct setup_t {
        float weight;
        std::string placement; // on_land_surface || partly_buried || on_ocean_floor || in_mountain || in_nether || underground
        float air_pocket_probability;
        float mossiness;
        bool overgrown;
        bool vines;
        bool can_be_cold;
        bool replace_with_blackstone;
    };

    resource_location_t type; // jigsaw || mineshaft || ...
    holder_set_t biomes;

    // one of monster, creature, ambient, water_creature, underground_water_creature, water_ambient, misc, or axolotls
    std::map<std::string, spawn_override_t> spawn_overrides = {}; // mob category -> { bounding_box, spawns }
    std::string step; // generation step
    std::string terrain_adaptation = "none"; // none || beard_thin || beard_box || bury || encapsulate
    // end_city -> no fields
    // igloo -> no fields
    // ocean_ruin ->
    std::optional<std::string> biome_temp; // normal (i hope it exists! minecraft wiki at moment i wrote this had nothing on that!) || cold || warm
    std::optional<float> cluster_probability;
    std::optional<float> large_probability;
    // nether_fossil || jigsaw ->
    std::optional<height_provider_t> start_height;
    // nether_fossil ->
    std::optional<height_provider_t> height;
    // ruined_portal ->
    std::optional<std::vector<setup_t>> setups;
    // shipwreck ->
    std::optional<bool> is_beached;
    // woodland_mansion -> no fields
    // buried_treasure -> no fields
    // desert_pyramid -> no fields
    // fortress -> no fields
    // mineshaft -> no fields
    std::optional<std::string> mineshaft_type; // normal || mesa
    // ocean_monument -> no fields
    // stronghold -> no fields
    // swamp_hut -> no fields
    // jigsaw ->
    std::optional<holder_t<template_pool_t>> start_pool;
    std::optional<int> size;
    std::optional<std::string> project_start_to_heightmap; // WORLD_SURFACE_WG || WORLD_SURFACE || OCEAN_FLOOR_WG || OCEAN_FLOOR || MOTION_BLOCKING || MOTION_BLOCKING_NO_LEAVES
    std::optional<std::string> start_jigsaw_name;

    struct max_distance_from_center_struct_t {
        int horizontal;
        std::optional<int> vertical;
    };
    struct dimension_padding_struct_t {
        int bottom = 0;
        int top = 0;
    };
    std::optional<std::variant<int, max_distance_from_center_struct_t>> max_distance_from_center;
    std::optional<bool> use_expansion_hack;
    std::optional<std::variant<int, dimension_padding_struct_t>> dimension_padding = 0;
    std::optional<std::string> liquid_settings; // apply_waterlogging || ignore_waterlogging

    struct pool_alias_t {
        struct target_t {
            float weight;
            resource_location_t data;
        };
        struct group_t {
            float weight;
            std::vector<pool_alias_t> data;
        };

        resource_location_t type;
        // random_group ->
        std::optional<std::vector<group_t>> groups;
        // random || direct ->
        std::optional<resource_location_t> alias;
        // random ->
        std::optional<std::vector<target_t>> targets;
        // direct ->
        std::optional<resource_location_t> target;
    };
    std::optional<std::vector<pool_alias_t>> pool_aliases;
};

}
