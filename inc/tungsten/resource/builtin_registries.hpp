/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include "registry.hpp"

#include <tungsten/world/worldgen/multi_noise_biome_source_parameter_list.hpp>
#include <tungsten/world/worldgen/flat_level_generator_preset.hpp>

#include <tungsten/world/entity/mob_variant.hpp>
#include <tungsten/world/entity/damage_type.hpp>

#include <tungsten/world/block/banner_pattern.hpp>
#include <tungsten/world/block/decorated_pot_pattern.hpp>

#include <tungsten/world/item/armor_trim.hpp>

#include <tungsten/world/gametest.hpp>

namespace tungsten {

namespace registry {
namespace worldgen {

extern registry_t<multi_noise_biome_source_parameter_list_t> multi_noise_biome_source_parameter_list;
extern registry_t<flat_level_generator_preset_t> flat_level_generator_preset;

}

extern registry_t<cat_variant_t> cat_variant;
extern registry_t<cow_variant_t> cow_variant;
extern registry_t<chicken_variant_t> chicken_variant;
extern registry_t<pig_variant_t> pig_variant;
extern registry_t<frog_variant_t> frog_variant;
extern registry_t<wolf_variant_t> wolf_variant;
extern registry_t<zombie_nautilus_variant_t> zombie_nautilus_variant;

extern registry_t<cat_sound_variant_t> cat_sound_variant;
extern registry_t<cow_sound_variant_t> cow_sound_variant;
extern registry_t<chicken_sound_variant_t> chicken_sound_variant;
extern registry_t<pig_sound_variant_t> pig_sound_variant;
extern registry_t<wolf_sound_variant_t> wolf_sound_variant;

extern registry_t<damage_type_t> damage_type;

extern registry_t<banner_pattern_t> banner_pattern;
extern registry_t<decorated_pot_pattern_t> decorated_pot_pattern;

extern registry_t<test_environment_t> test_environment;
extern registry_t<test_instance_t> test_instance;

extern registry_t<trim_material_t> trim_material;
extern registry_t<trim_pattern_t> trim_pattern;

}

void parse_builtin(std::string_view path);

}
