/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <optional>
#include <string>
#include <vector>
#include <tungsten/resource/holder_set.hpp>
#include <tungsten/resource/resource_location.hpp>
#include <tungsten/type/sound_event.hpp>
#include <tungsten/util/json_reflect.hpp>
#include <tungsten/util/json_range.hpp>
#include <tungsten/world/attribute/attribute_modifier.hpp>

namespace tungsten {

struct spawn_condition_t {
    struct biome_t {
        static constexpr std::string_view type_tag = "biome";
        holder_set_t biomes;
    };
    struct structure_t {
        static constexpr std::string_view type_tag = "structure";
        holder_set_t structures;
    };
    struct moon_brightness_t {
        static constexpr std::string_view type_tag = "moon_brightness";
        double_range_t range;
    };
    using condition_t = std::variant<biome_t, structure_t, moon_brightness_t>;

    int priority;
    std::optional<condition_t> condition;
};

struct cat_variant_t {
    resource_location_t asset_id;
    resource_location_t baby_asset_id;
    std::vector<spawn_condition_t> spawn_conditions;
};
struct chicken_variant_t {
    resource_location_t asset_id;
    resource_location_t baby_asset_id;
    std::string model = "normal"; // normal || cold
    std::vector<spawn_condition_t> spawn_conditions;
};
struct cow_variant_t {
    resource_location_t asset_id;
    resource_location_t baby_asset_id;
    std::string model = "normal"; // normal || cold || warm
    std::vector<spawn_condition_t> spawn_conditions;
};
struct frog_variant_t {
    resource_location_t asset_id;
    std::vector<spawn_condition_t> spawn_conditions;
};
struct pig_variant_t {
    resource_location_t asset_id;
    resource_location_t baby_asset_id;
    std::string model = "normal"; // normal || cold
    std::vector<spawn_condition_t> spawn_conditions;
};
struct wolf_variant_t {
    struct sections_t {
        resource_location_t angry;
        resource_location_t wild;
        resource_location_t tame;
    };

    sections_t assets;
    sections_t baby_assets;
    std::vector<spawn_condition_t> spawn_conditions;
};
struct zombie_nautilus_variant_t {
    resource_location_t asset_id;
    std::string model = "normal"; // normal || warm
    std::vector<spawn_condition_t> spawn_conditions;
};

struct cat_sound_variant_t {
    struct sections_t {
        std::optional<sound_event_t> ambient_sound;
        std::optional<sound_event_t> beg_for_food_sound;
        std::optional<sound_event_t> death_sound;
        std::optional<sound_event_t> eat_sound;
        std::optional<sound_event_t> hiss_sound;
        std::optional<sound_event_t> hurt_sound;
        std::optional<sound_event_t> purr_sound;
        std::optional<sound_event_t> purreow_sound;
        std::optional<sound_event_t> stray_ambient_sound;
    };

    sections_t adult_sounds;
    sections_t baby_sounds;
};
struct chicken_sound_variant_t {
    struct sections_t {
        std::optional<sound_event_t> ambient_sound;
        std::optional<sound_event_t> death_sound;
        std::optional<sound_event_t> hurt_sound;
        std::optional<sound_event_t> step_sound;
    };

    sections_t adult_sounds;
    sections_t baby_sounds;
};
struct cow_sound_variant_t {
    std::optional<sound_event_t> ambient_sound;
    std::optional<sound_event_t> death_sound;
    std::optional<sound_event_t> hurt_sound;
    std::optional<sound_event_t> step_sound;
};
struct pig_sound_variant_t {
    struct sections_t {
        std::optional<sound_event_t> ambient_sound;
        std::optional<sound_event_t> death_sound;
        std::optional<sound_event_t> eat_sound;
        std::optional<sound_event_t> hurt_sound;
        std::optional<sound_event_t> step_sound;
    };

    sections_t adult_sounds;
    sections_t baby_sounds;
};
struct wolf_sound_variant_t {
    struct sections_t {
        std::optional<sound_event_t> ambient_sound;
        std::optional<sound_event_t> death_sound;
        std::optional<sound_event_t> growl_sound;
        std::optional<sound_event_t> hurt_sound;
        std::optional<sound_event_t> pant_sound;
        std::optional<sound_event_t> whine_sound;
        std::optional<sound_event_t> step_sound;
    };

    sections_t adult_sounds;
    sections_t baby_sounds;
};

struct sulfur_cube_archetype_t {
    struct knockback_modifiers_t {
        float horizontal_power;
        float vertical_power;
    };
    struct sound_settings_t {
        std::optional<sound_event_t> hit_sound;
        std::optional<sound_event_t> push_sound;
        float push_sound_cooldown;
        float push_sound_impulse_threshold;
    };
    struct explosion_t {
        bool causes_fire = false;
        int fuse;
        int power;
    };
    struct contact_damage_t {
        float amount;
        bool attribute_to_source = false;
        resource_location_t damage_type;
    };

    std::vector<attribute_modifier_t> attribute_modifiers = {};
    bool buoyant = false;
    std::optional<contact_damage_t> contact_damage;
    std::optional<explosion_t> explosion;
    holder_set_t items;
    knockback_modifiers_t knockback_modifiers;
    sound_settings_t sound_settings;
    // location_based_effects​[upcoming: JE 26.4]: Optional; a list of location-based effects
};

}
