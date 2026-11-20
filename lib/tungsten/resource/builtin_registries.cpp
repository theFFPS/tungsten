/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/resource/builtin_registries.hpp>

namespace tungsten {

namespace registry {

registry_t<advancement_t> advancement { resource_location_t("advancement") };
registry_t<banner_pattern_t> banner_pattern { resource_location_t("banner_pattern") };
registry_t<block_transformer_t> block_transformer { resource_location_t("block_transformer") };
registry_t<cat_variant_t> cat_variant { resource_location_t("cat_variant") };
registry_t<chat_type_t> chat_type { resource_location_t("chat_type") };
registry_t<chicken_variant_t> chicken_variant { resource_location_t("chicken_variant") };
registry_t<context_number_provider_t> context_float_provider { resource_location_t("context_float_provider") };
registry_t<context_number_provider_t> context_int_provider { resource_location_t("context_int_provider") };
registry_t<cow_variant_t> cow_variant { resource_location_t("cow_variant") };
registry_t<damage_type_t> damage_type { resource_location_t("damage_type") };
registry_t<decorated_pot_pattern_t> decorated_pot_pattern { resource_location_t("decorated_pot_pattern") };
registry_t<dialog_t> dialog { resource_location_t("dialog") };
registry_t<dimension_t> dimension { resource_location_t("dimension") };
registry_t<dimension_type_t> dimension_type { resource_location_t("dimension_type") };
registry_t<enchantment_t> enchantment { resource_location_t("enchantment") };
registry_t<enchantment_provider_t> enchantment_provider { resource_location_t("enchantment_provider") };
registry_t<frog_variant_t> frog_variant { resource_location_t("frog_variant") };
registry_t<function_t> function { resource_location_t("function") };
registry_t<instrument_t> instrument { resource_location_t("instrument") };
registry_t<item_modifier_t> item_modifier { resource_location_t("item_modifier") };
registry_t<jukebox_song_t> jukebox_song { resource_location_t("jukebox_song") };
registry_t<loot_table_t> loot_table { resource_location_t("loot_table") };
registry_t<painting_variant_t> painting_variant { resource_location_t("painting_variant") };
registry_t<pig_variant_t> pig_variant { resource_location_t("pig_variant") };
registry_t<predicate_t> predicate { resource_location_t("predicate") };
registry_t<recipe_t> recipe { resource_location_t("recipe") };
registry_t<raw_resource_t> slot_source { resource_location_t("slot_source") };
registry_t<structure_template_t> structure { resource_location_t("structure") };
registry_t<sulfur_cube_archetype_t> sulfur_cube_archetype { resource_location_t("sulfur_cube_archetype") };
registry_t<test_environment_t> test_environment { resource_location_t("test_environment") };
registry_t<test_instance_t> test_instance { resource_location_t("test_instance") };
registry_t<timeline_t> timeline { resource_location_t("timeline") };
registry_t<trade_set_t> trade_set { resource_location_t("trade_set") };
registry_t<trial_spawner_t> trial_spawner { resource_location_t("trial_spawner") };
registry_t<trim_material_t> trim_material { resource_location_t("trim_material") };
registry_t<trim_pattern_t> trim_pattern { resource_location_t("trim_pattern") };
registry_t<villager_trade_t> villager_trade { resource_location_t("villager_trade") };
registry_t<wolf_variant_t> wolf_variant { resource_location_t("wolf_variant") };
registry_t<world_clock_t> world_clock { resource_location_t("world_clock") };
registry_t<zombie_nautilus_variant_t> zombie_nautilus_variant { resource_location_t("zombie_nautilus_variant") };
registry_t<cat_sound_variant_t> cat_sound_variant { resource_location_t("cat_sound_variant") };
registry_t<chicken_sound_variant_t> chicken_sound_variant { resource_location_t("chicken_sound_variant") };
registry_t<cow_sound_variant_t> cow_sound_variant { resource_location_t("cow_sound_variant") };
registry_t<pig_sound_variant_t> pig_sound_variant { resource_location_t("pig_sound_variant") };
registry_t<wolf_sound_variant_t> wolf_sound_variant { resource_location_t("wolf_sound_variant") };

namespace worldgen {

registry_t<biome_t> biome { resource_location_t("worldgen/biome") };
registry_t<block_state_provider_t> block_state_provider { resource_location_t("worldgen/block_state_provider") };
registry_t<configured_carver_t> carver { resource_location_t("worldgen/carver") };
registry_t<density_function_t> density_function { resource_location_t("worldgen/density_function") };
registry_t<configured_feature_t> feature { resource_location_t("worldgen/feature") };
registry_t<flat_level_generator_preset_t> flat_level_generator_preset { resource_location_t("worldgen/flat_level_generator_preset") };
registry_t<material_condition_t> material_condition { resource_location_t("worldgen/material_condition") };
registry_t<material_rule_t> material_rule { resource_location_t("worldgen/material_rule") };
registry_t<multi_noise_biome_source_parameter_list_t> multi_noise_biome_source_parameter_list { resource_location_t("worldgen/multi_noise_biome_source_parameter_list") };
registry_t<noise_parameters_t> noise { resource_location_t("worldgen/noise") };
registry_t<noise_settings_t> noise_settings { resource_location_t("worldgen/noise_settings") };
registry_t<placed_feature_t> placed_feature { resource_location_t("worldgen/placed_feature") };
registry_t<processor_list_t> processor_list { resource_location_t("worldgen/processor_list") };
registry_t<structure_t> structure { resource_location_t("worldgen/structure") };
registry_t<structure_set_t> structure_set { resource_location_t("worldgen/structure_set") };
registry_t<template_pool_t> template_pool { resource_location_t("worldgen/template_pool") };
registry_t<world_preset_t> world_preset { resource_location_t("worldgen/world_preset") };

}

const std::vector<generic_registry_t*>& all() {
    static const std::vector<generic_registry_t*> registries = {
        &advancement,
        &banner_pattern,
        &block_transformer,
        &cat_variant,
        &chat_type,
        &chicken_variant,
        &context_float_provider,
        &context_int_provider,
        &cow_variant,
        &damage_type,
        &decorated_pot_pattern,
        &dialog,
        &dimension,
        &dimension_type,
        &enchantment,
        &enchantment_provider,
        &frog_variant,
        &function,
        &instrument,
        &item_modifier,
        &jukebox_song,
        &loot_table,
        &painting_variant,
        &pig_variant,
        &predicate,
        &recipe,
        &slot_source,
        &structure,
        &sulfur_cube_archetype,
        &test_environment,
        &test_instance,
        &timeline,
        &trade_set,
        &trial_spawner,
        &trim_material,
        &trim_pattern,
        &villager_trade,
        &wolf_variant,
        &world_clock,
        &zombie_nautilus_variant,
        &cat_sound_variant,
        &chicken_sound_variant,
        &cow_sound_variant,
        &pig_sound_variant,
        &wolf_sound_variant,
        &worldgen::biome,
        &worldgen::block_state_provider,
        &worldgen::carver,
        &worldgen::density_function,
        &worldgen::feature,
        &worldgen::flat_level_generator_preset,
        &worldgen::material_condition,
        &worldgen::material_rule,
        &worldgen::multi_noise_biome_source_parameter_list,
        &worldgen::noise,
        &worldgen::noise_settings,
        &worldgen::placed_feature,
        &worldgen::processor_list,
        &worldgen::structure,
        &worldgen::structure_set,
        &worldgen::template_pool,
        &worldgen::world_preset
    };
    return registries;
}
generic_registry_t* by_name(std::string_view directory) {
    for (auto* registry : all()) {
        if (registry->identifier().m_path == directory) return registry;
    }
    return nullptr;
}
void clear_all() {
    for (auto* registry : all()) registry->clear();
}

}

}
