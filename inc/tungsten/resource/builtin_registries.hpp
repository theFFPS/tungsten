/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include "registry.hpp"
#include <tungsten/client/dialog.hpp>
#include <tungsten/world/time.hpp>
#include <tungsten/world/dimension.hpp>
#include <tungsten/world/advancement.hpp>
#include <tungsten/world/chat_type.hpp>
#include <tungsten/world/loot_table.hpp>
#include <tungsten/world/context_number_provider.hpp>
#include <tungsten/world/item/enchantment.hpp>
#include <tungsten/world/item/recipe.hpp>
#include <tungsten/world/item/sound_item.hpp>
#include <tungsten/world/entity/villager_trade.hpp>
#include <tungsten/world/entity/painting_variant.hpp>
#include <tungsten/world/block/trial_spawner.hpp>
#include <tungsten/world/block/block_transformer.hpp>
#include <tungsten/world/function.hpp>
#include <tungsten/world/structure_template.hpp>
#include <tungsten/world/worldgen/flat_level_generator_preset.hpp>
#include <tungsten/world/worldgen/multi_noise_biome_source_parameter_list.hpp>
#include <tungsten/world/worldgen/biome.hpp>
#include <tungsten/world/worldgen/carver.hpp>
#include <tungsten/world/worldgen/feature.hpp>
#include <tungsten/world/worldgen/noise.hpp>
#include <tungsten/world/worldgen/processor_list.hpp>
#include <tungsten/world/worldgen/structure.hpp>
#include <tungsten/world/worldgen/structure_set.hpp>
#include <tungsten/world/worldgen/template_pool.hpp>
#include <tungsten/world/worldgen/world_preset.hpp>
#include <tungsten/resource/raw_resource.hpp>
#include <tungsten/world/entity/mob_variant.hpp>
#include <tungsten/world/entity/damage_type.hpp>
#include <tungsten/world/block/banner_pattern.hpp>
#include <tungsten/world/block/decorated_pot_pattern.hpp>
#include <tungsten/world/item/armor_trim.hpp>
#include <tungsten/world/gametest.hpp>

namespace tungsten {

// .nbt structure files and .mcfunction files are not JSON
template<> struct registry_codec_t<structure_template_t> {
    static constexpr std::string_view extension = ".nbt";
    static structure_template_t decode(std::span<const std::uint8_t> bytes, const resource_location_t&) {
        return structure_template_t::parse(bytes);
    }
};
template<> struct registry_codec_t<function_t> {
    static constexpr std::string_view extension = ".mcfunction";
    static function_t decode(std::span<const std::uint8_t> bytes, const resource_location_t&) {
        return function_t::parse(bytes);
    }
};

// Every registry which is read from data packs. Registries are process wide singletons (like the game's built-in registries) and are
// filled by datapack_manager_t.
namespace registry {

extern registry_t<advancement_t> advancement;
extern registry_t<banner_pattern_t> banner_pattern;
extern registry_t<block_transformer_t> block_transformer;
extern registry_t<cat_variant_t> cat_variant;
extern registry_t<chat_type_t> chat_type;
extern registry_t<chicken_variant_t> chicken_variant;
extern registry_t<context_number_provider_t> context_float_provider;
extern registry_t<context_number_provider_t> context_int_provider;
extern registry_t<cow_variant_t> cow_variant;
extern registry_t<damage_type_t> damage_type;
extern registry_t<decorated_pot_pattern_t> decorated_pot_pattern;
extern registry_t<dialog_t> dialog;
extern registry_t<dimension_t> dimension;
extern registry_t<dimension_type_t> dimension_type;
extern registry_t<enchantment_t> enchantment;
extern registry_t<enchantment_provider_t> enchantment_provider;
extern registry_t<frog_variant_t> frog_variant;
extern registry_t<function_t> function;
extern registry_t<instrument_t> instrument;
extern registry_t<item_modifier_t> item_modifier;
extern registry_t<jukebox_song_t> jukebox_song;
extern registry_t<loot_table_t> loot_table;
extern registry_t<painting_variant_t> painting_variant;
extern registry_t<pig_variant_t> pig_variant;
extern registry_t<predicate_t> predicate;
extern registry_t<recipe_t> recipe;
extern registry_t<raw_resource_t> slot_source;
extern registry_t<structure_template_t> structure;
extern registry_t<sulfur_cube_archetype_t> sulfur_cube_archetype;
extern registry_t<test_environment_t> test_environment;
extern registry_t<test_instance_t> test_instance;
extern registry_t<timeline_t> timeline;
extern registry_t<trade_set_t> trade_set;
extern registry_t<trial_spawner_t> trial_spawner;
extern registry_t<trim_material_t> trim_material;
extern registry_t<trim_pattern_t> trim_pattern;
extern registry_t<villager_trade_t> villager_trade;
extern registry_t<wolf_variant_t> wolf_variant;
extern registry_t<world_clock_t> world_clock;
extern registry_t<zombie_nautilus_variant_t> zombie_nautilus_variant;

extern registry_t<cat_sound_variant_t> cat_sound_variant;
extern registry_t<chicken_sound_variant_t> chicken_sound_variant;
extern registry_t<cow_sound_variant_t> cow_sound_variant;
extern registry_t<pig_sound_variant_t> pig_sound_variant;
extern registry_t<wolf_sound_variant_t> wolf_sound_variant;

namespace worldgen {

extern registry_t<biome_t> biome;
extern registry_t<configured_carver_t> carver;
extern registry_t<density_function_t> density_function;
extern registry_t<configured_feature_t> feature;
extern registry_t<flat_level_generator_preset_t> flat_level_generator_preset;
extern registry_t<multi_noise_biome_source_parameter_list_t> multi_noise_biome_source_parameter_list;
extern registry_t<noise_t> noise;
extern registry_t<noise_settings_t> noise_settings;
extern registry_t<placed_feature_t> placed_feature;
extern registry_t<processor_list_t> processor_list;
extern registry_t<structure_t> structure;
extern registry_t<structure_set_t> structure_set;
extern registry_t<template_pool_t> template_pool;
extern registry_t<world_preset_t> world_preset;

}

// every registry above, in dependency-free alphabetical order
const std::vector<generic_registry_t*>& all();
generic_registry_t* by_name(std::string_view directory); // nullptr when unknown, e.g. "worldgen/biome"
void clear_all();

}

}
