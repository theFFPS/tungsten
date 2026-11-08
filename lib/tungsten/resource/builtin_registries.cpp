/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/resource/resource_location.hpp>
#include <tungsten/resource/builtin_registries.hpp>

namespace tungsten {

namespace registry {
namespace worldgen {

registry_t<multi_noise_biome_source_parameter_list_t> multi_noise_biome_source_parameter_list
    = registry_t<multi_noise_biome_source_parameter_list_t>(resource_location_t("worldgen/multi_noise_biome_source_parameter_list"));
registry_t<flat_level_generator_preset_t> flat_level_generator_preset = registry_t<flat_level_generator_preset_t>(resource_location_t("worldgen/flat_level_generator_preset"));

}

registry_t<cat_variant_t> cat_variant = registry_t<cat_variant_t>(resource_location_t("cat_variant"));
registry_t<cow_variant_t> cow_variant = registry_t<cow_variant_t>(resource_location_t("cow_variant"));
registry_t<chicken_variant_t> chicken_variant = registry_t<chicken_variant_t>(resource_location_t("chicken_variant"));
registry_t<pig_variant_t> pig_variant = registry_t<pig_variant_t>(resource_location_t("pig_variant"));
registry_t<frog_variant_t> frog_variant = registry_t<frog_variant_t>(resource_location_t("frog_variant"));
registry_t<wolf_variant_t> wolf_variant = registry_t<wolf_variant_t>(resource_location_t("wolf_variant"));
registry_t<zombie_nautilus_variant_t> zombie_nautilus_variant = registry_t<zombie_nautilus_variant_t>(resource_location_t("zombie_nautilus_variant"));

registry_t<cat_sound_variant_t> cat_sound_variant = registry_t<cat_sound_variant_t>(resource_location_t("cat_sound_variant"));
registry_t<cow_sound_variant_t> cow_sound_variant = registry_t<cow_sound_variant_t>(resource_location_t("cow_sound_variant"));
registry_t<chicken_sound_variant_t> chicken_sound_variant = registry_t<chicken_sound_variant_t>(resource_location_t("chicken_sound_variant"));
registry_t<pig_sound_variant_t> pig_sound_variant = registry_t<pig_sound_variant_t>(resource_location_t("pig_sound_variant"));
registry_t<wolf_sound_variant_t> wolf_sound_variant = registry_t<wolf_sound_variant_t>(resource_location_t("wolf_sound_variant"));

registry_t<damage_type_t> damage_type = registry_t<damage_type_t>(resource_location_t("damage_type"));

registry_t<banner_pattern_t> banner_pattern = registry_t<banner_pattern_t>(resource_location_t("banner_pattern"));
registry_t<decorated_pot_pattern_t> decorated_pot_pattern = registry_t<decorated_pot_pattern_t>(resource_location_t("decorated_pot_pattern"));

registry_t<trim_material_t> trim_material = registry_t<trim_material_t>(resource_location_t("trim_material"));
registry_t<trim_pattern_t> trim_pattern = registry_t<trim_pattern_t>(resource_location_t("trim_pattern"));

registry_t<test_environment_t> test_environment = registry_t<test_environment_t>(resource_location_t("test_environment"));
registry_t<test_instance_t> test_instance = registry_t<test_instance_t>(resource_location_t("test_instance"));

}

void parse_builtin(std::string_view path) {
    registry::worldgen::multi_noise_biome_source_parameter_list.read(path);
    registry::worldgen::flat_level_generator_preset.read(path);

    registry::cat_variant.read(path);
    registry::cow_variant.read(path);
    registry::chicken_variant.read(path);
    registry::pig_variant.read(path);
    registry::frog_variant.read(path);
    registry::wolf_variant.read(path);
    registry::zombie_nautilus_variant.read(path);

    registry::cat_sound_variant.read(path);
    registry::cow_sound_variant.read(path);
    registry::chicken_sound_variant.read(path);
    registry::pig_sound_variant.read(path);
    registry::wolf_sound_variant.read(path);

    registry::damage_type.read(path);

    registry::banner_pattern.read(path);
    registry::decorated_pot_pattern.read(path);

    registry::trim_material.read(path);
    registry::trim_pattern.read(path);

    registry::test_environment.read(path);
    registry::test_instance.read(path);
}

}
