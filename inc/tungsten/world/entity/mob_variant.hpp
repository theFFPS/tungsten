/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/type/variant.hpp>
#include <tungsten/type/sound_event.hpp>
#include <tungsten/resource/resource_location.hpp>

namespace tungsten {

struct spawn_condition_t {
    struct condition_t {
        struct moon_brightness_range_t {
            std::optional<double> m_min;
            std::optional<double> m_max;

            friend void to_json(nlohmann::json& j, const spawn_condition_t::condition_t::moon_brightness_range_t& val) {
                if (val.m_max.has_value() && val.m_min.has_value()) {
                    if (val.m_max.value() == val.m_min.value()) j = val.m_max.value();
                    else j = {
                        { "max", val.m_max.value() },
                        { "min", val.m_min.value() }
                    };
                } else if (val.m_max.has_value()) {
                    j = { { "max", val.m_max.value() } };
                } else if (val.m_min.has_value()) {
                    j = { { "min", val.m_min.value() } };
                }
            }
            friend void from_json(const nlohmann::json& j, spawn_condition_t::condition_t::moon_brightness_range_t& val) {
                if (j.is_number()) {
                    val.m_max = (double) j;
                    val.m_min = (double) j;
                } else {
                    if (j.contains("max")) val.m_max = (double) j["max"];
                    if (j.contains("min")) val.m_min = (double) j["min"];
                }
            }
        };

        resource_location_t m_type;
        std::variant<
            moon_brightness_range_t,
            std::string, std::vector<resource_location_t> // m_biomes || m_structures
        > m_value;

        friend void to_json(nlohmann::json& j, const spawn_condition_t::condition_t& val) {
            std::visit(overloaded{
                [&j, type = val.m_type](const moon_brightness_range_t& value) {
                    j = {
                        { "type", type },
                        { "range", value }
                    };
                },
                [&j, type = val.m_type](const std::string& value) {
                    if (type == resource_location_t("biome")) j = {
                        { "type", type },
                        { "biomes", value }
                    }; else j = {
                        { "type", type },
                        { "structures", value }
                    };
                },
                [&j, type = val.m_type](const std::vector<resource_location_t>& value) {
                    if (type == resource_location_t("biome")) j = {
                        { "type", type },
                        { "biomes", value }
                    }; else j = {
                        { "type", type },
                        { "structures", value }
                    };
                }
            }, val.m_value);
        }
        friend void from_json(const nlohmann::json& j, spawn_condition_t::condition_t& val) {
            val.m_type = j["type"];
            if (val.m_type.to_string() == "minecraft:moon_brightness") val.m_value = (moon_brightness_range_t) j["range"];
            else if (val.m_type.to_string() == "minecraft:biome") {
                if (j["biomes"].is_string()) val.m_value = (std::string) j["biomes"];
                else val.m_value = std::vector<resource_location_t>(j["biomes"]);
            } else if (val.m_type.to_string() == "minecraft:structure") {
                if (j["structures"].is_string()) val.m_value = (std::string) j["structures"];
                else val.m_value = std::vector<resource_location_t>(j["structures"]);
            }
        }
    };

    int m_priority;
    std::optional<condition_t> m_condition;

    friend void to_json(nlohmann::json& j, const spawn_condition_t& val) {
        if (val.m_condition.has_value()) j = {
            { "priority", val.m_priority },
            { "condition", val.m_condition.value() }
        };
        else j = { { "priority", val.m_priority } };
    }
    friend void from_json(const nlohmann::json& j, spawn_condition_t& val) {
        val.m_priority = j["priority"];
        if (j.contains("condition")) val.m_condition = condition_t(j["condition"]);
    }
};

struct cat_variant_t {
    resource_location_t asset_id;
    resource_location_t baby_asset_id;
    std::vector<spawn_condition_t> spawn_conditions;
};
struct chicken_variant_t {
    resource_location_t m_asset_id;
    resource_location_t m_baby_asset_id;
    std::optional<resource_location_t> m_model; // normal || cold
    std::vector<spawn_condition_t> m_spawn_conditions;

    friend void to_json(nlohmann::json& j, const chicken_variant_t& val) {
        j = {
            { "asset_id", val.m_asset_id },
            { "baby_asset_id", val.m_baby_asset_id },
            { "model", val.m_model.value_or(resource_location_t("normal")).m_path },
            { "spawn_conditions", val.m_spawn_conditions }
        };
    }
    friend void from_json(const nlohmann::json& j, chicken_variant_t& val) {
        val.m_asset_id = j["asset_id"];
        val.m_baby_asset_id = j["baby_asset_id"];
        val.m_spawn_conditions = j["spawn_conditions"];
        if (j.contains("model")) val.m_model = resource_location_t(j["model"]);
    }
};
struct cow_variant_t {
    resource_location_t m_asset_id;
    resource_location_t m_baby_asset_id;
    std::optional<resource_location_t> m_model; // normal || cold || warm
    std::vector<spawn_condition_t> m_spawn_conditions;

    friend void to_json(nlohmann::json& j, const cow_variant_t& val) {
        j = {
            { "asset_id", val.m_asset_id },
            { "baby_asset_id", val.m_baby_asset_id },
            { "model", val.m_model.value_or(resource_location_t("normal")).m_path },
            { "spawn_conditions", val.m_spawn_conditions }
        };
    }
    friend void from_json(const nlohmann::json& j, cow_variant_t& val) {
        val.m_asset_id = j["asset_id"];
        val.m_baby_asset_id = j["baby_asset_id"];
        val.m_spawn_conditions = j["spawn_conditions"];
        if (j.contains("model")) val.m_model = resource_location_t(j["model"]);
    }
};
struct frog_variant_t {
    resource_location_t asset_id;
    std::vector<spawn_condition_t> spawn_conditions;
};
struct pig_variant_t {
    resource_location_t m_asset_id;
    resource_location_t m_baby_asset_id;
    std::optional<resource_location_t> m_model; // normal || cold
    std::vector<spawn_condition_t> m_spawn_conditions;

    friend void to_json(nlohmann::json& j, const pig_variant_t& val) {
        j = {
            { "asset_id", val.m_asset_id },
            { "baby_asset_id", val.m_baby_asset_id },
            { "model", val.m_model.value_or(resource_location_t("normal")).m_path },
            { "spawn_conditions", val.m_spawn_conditions }
        };
    }
    friend void from_json(const nlohmann::json& j, pig_variant_t& val) {
        val.m_asset_id = j["asset_id"];
        val.m_baby_asset_id = j["baby_asset_id"];
        val.m_spawn_conditions = j["spawn_conditions"];
        if (j.contains("model")) val.m_model = resource_location_t(j["model"]);
    }
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
    resource_location_t m_asset_id;
    std::optional<resource_location_t> m_model; // normal || warm
    std::vector<spawn_condition_t> m_spawn_conditions;

    friend void to_json(nlohmann::json& j, const zombie_nautilus_variant_t& val) {
        j = {
            { "asset_id", val.m_asset_id },
            { "model", val.m_model.value_or(resource_location_t("normal")).m_path },
            { "spawn_conditions", val.m_spawn_conditions }
        };
    }
    friend void from_json(const nlohmann::json& j, zombie_nautilus_variant_t& val) {
        val.m_asset_id = j["asset_id"];
        val.m_spawn_conditions = j["spawn_conditions"];
        if (j.contains("model")) val.m_model = resource_location_t(j["model"]);
    }
};

struct cat_sound_variant_t {
    struct sections_t {
        std::optional<sound_event_t> m_ambient_sound;
        std::optional<sound_event_t> m_beg_for_food_sound;
        std::optional<sound_event_t> m_death_sound;
        std::optional<sound_event_t> m_eat_sound;
        std::optional<sound_event_t> m_hiss_sound;
        std::optional<sound_event_t> m_hurt_sound;
        std::optional<sound_event_t> m_purr_sound;
        std::optional<sound_event_t> m_purreow_sound;
        std::optional<sound_event_t> m_stray_ambient_sound;

        friend void to_json(nlohmann::json& j, const cat_sound_variant_t::sections_t& val) {
            std::unordered_map<std::string, sound_event_t> entries;
            if (val.m_ambient_sound.has_value()) entries.insert({ "ambient_sound", val.m_ambient_sound.value() });
            if (val.m_beg_for_food_sound.has_value()) entries.insert({ "beg_for_food_sound", val.m_beg_for_food_sound.value() });
            if (val.m_death_sound.has_value()) entries.insert({ "death_sound", val.m_death_sound.value() });
            if (val.m_eat_sound.has_value()) entries.insert({ "eat_sound", val.m_eat_sound.value() });
            if (val.m_hiss_sound.has_value()) entries.insert({ "hiss_sound", val.m_hiss_sound.value() });
            if (val.m_hurt_sound.has_value()) entries.insert({ "hurt_sound", val.m_hurt_sound.value() });
            if (val.m_purr_sound.has_value()) entries.insert({ "purr_sound", val.m_purr_sound.value() });
            if (val.m_purreow_sound.has_value()) entries.insert({ "purreow_sound", val.m_purreow_sound.value() });
            if (val.m_stray_ambient_sound.has_value()) entries.insert({ "stray_ambient_sound", val.m_stray_ambient_sound.value() });
            j = entries;
        }
        friend void from_json(const nlohmann::json& j, cat_sound_variant_t::sections_t& val) {
            if (j.contains("ambient_sound")) val.m_ambient_sound = sound_event_t(j["ambient_sound"]);
            if (j.contains("beg_for_food_sound")) val.m_beg_for_food_sound = sound_event_t(j["beg_for_food_sound"]);
            if (j.contains("death_sound")) val.m_death_sound = sound_event_t(j["death_sound"]);
            if (j.contains("eat_sound")) val.m_eat_sound = sound_event_t(j["eat_sound"]);
            if (j.contains("hiss_sound")) val.m_hiss_sound = sound_event_t(j["hiss_sound"]);
            if (j.contains("hurt_sound")) val.m_hurt_sound = sound_event_t(j["hurt_sound"]);
            if (j.contains("purreow_sound")) val.m_purreow_sound = sound_event_t(j["purreow_sound"]);
            if (j.contains("stray_ambient_sound")) val.m_stray_ambient_sound = sound_event_t(j["stray_ambient_sound"]);
        }
    };

    sections_t adult_sounds;
    sections_t baby_sounds;
};
struct chicken_sound_variant_t {
    struct sections_t {
        std::optional<sound_event_t> m_ambient_sound;
        std::optional<sound_event_t> m_death_sound;
        std::optional<sound_event_t> m_hurt_sound;
        std::optional<sound_event_t> m_step_sound;

        friend void to_json(nlohmann::json& j, const chicken_sound_variant_t::sections_t& val) {
            std::unordered_map<std::string, sound_event_t> entries;
            if (val.m_ambient_sound.has_value()) entries.insert({ "ambient_sound", val.m_ambient_sound.value() });
            if (val.m_death_sound.has_value()) entries.insert({ "death_sound", val.m_death_sound.value() });
            if (val.m_hurt_sound.has_value()) entries.insert({ "hurt_sound", val.m_hurt_sound.value() });
            if (val.m_step_sound.has_value()) entries.insert({ "step_sound", val.m_step_sound.value() });
            j = entries;
        }
        friend void from_json(const nlohmann::json& j, chicken_sound_variant_t::sections_t& val) {
            if (j.contains("ambient_sound")) val.m_ambient_sound = sound_event_t(j["ambient_sound"]);
            if (j.contains("death_sound")) val.m_death_sound = sound_event_t(j["death_sound"]);
            if (j.contains("hurt_sound")) val.m_hurt_sound = sound_event_t(j["hurt_sound"]);
            if (j.contains("step_sound")) val.m_step_sound = sound_event_t(j["step_sound"]);
        }
    };

    sections_t adult_sounds;
    sections_t baby_sounds;
};
struct cow_sound_variant_t {
    std::optional<sound_event_t> m_ambient_sound;
    std::optional<sound_event_t> m_death_sound;
    std::optional<sound_event_t> m_hurt_sound;
    std::optional<sound_event_t> m_step_sound;

    friend void to_json(nlohmann::json& j, const cow_sound_variant_t& val) {
        std::unordered_map<std::string, sound_event_t> entries;
        if (val.m_ambient_sound.has_value()) entries.insert({ "ambient_sound", val.m_ambient_sound.value() });
        if (val.m_death_sound.has_value()) entries.insert({ "death_sound", val.m_death_sound.value() });
        if (val.m_hurt_sound.has_value()) entries.insert({ "hurt_sound", val.m_hurt_sound.value() });
        if (val.m_step_sound.has_value()) entries.insert({ "step_sound", val.m_step_sound.value() });
        j = entries;
    }
    friend void from_json(const nlohmann::json& j, cow_sound_variant_t& val) {
        if (j.contains("ambient_sound")) val.m_ambient_sound = sound_event_t(j["ambient_sound"]);
        if (j.contains("death_sound")) val.m_death_sound = sound_event_t(j["death_sound"]);
        if (j.contains("hurt_sound")) val.m_hurt_sound = sound_event_t(j["hurt_sound"]);
        if (j.contains("step_sound")) val.m_step_sound = sound_event_t(j["step_sound"]);
    }
};
struct pig_sound_variant_t {
    struct sections_t {
        std::optional<sound_event_t> m_ambient_sound;
        std::optional<sound_event_t> m_death_sound;
        std::optional<sound_event_t> m_eat_sound;
        std::optional<sound_event_t> m_hurt_sound;
        std::optional<sound_event_t> m_step_sound;

        friend void to_json(nlohmann::json& j, const pig_sound_variant_t::sections_t& val) {
            std::unordered_map<std::string, sound_event_t> entries;
            if (val.m_ambient_sound.has_value()) entries.insert({ "ambient_sound", val.m_ambient_sound.value() });
            if (val.m_death_sound.has_value()) entries.insert({ "death_sound", val.m_death_sound.value() });
            if (val.m_hurt_sound.has_value()) entries.insert({ "hurt_sound", val.m_hurt_sound.value() });
            if (val.m_eat_sound.has_value()) entries.insert({ "eat_sound", val.m_eat_sound.value() });
            if (val.m_step_sound.has_value()) entries.insert({ "step_sound", val.m_step_sound.value() });
            j = entries;
        }
        friend void from_json(const nlohmann::json& j, pig_sound_variant_t::sections_t& val) {
            if (j.contains("ambient_sound")) val.m_ambient_sound = sound_event_t(j["ambient_sound"]);
            if (j.contains("death_sound")) val.m_death_sound = sound_event_t(j["death_sound"]);
            if (j.contains("hurt_sound")) val.m_hurt_sound = sound_event_t(j["hurt_sound"]);
            if (j.contains("eat_sound")) val.m_eat_sound = sound_event_t(j["eat_sound"]);
            if (j.contains("step_sound")) val.m_step_sound = sound_event_t(j["step_sound"]);
        }
    };

    sections_t adult_sounds;
    sections_t baby_sounds;
};
struct wolf_sound_variant_t {
    struct sections_t {
        std::optional<sound_event_t> m_ambient_sound;
        std::optional<sound_event_t> m_death_sound;
        std::optional<sound_event_t> m_growl_sound;
        std::optional<sound_event_t> m_hurt_sound;
        std::optional<sound_event_t> m_pant_sound;
        std::optional<sound_event_t> m_whine_sound;
        std::optional<sound_event_t> m_step_sound;

        friend void to_json(nlohmann::json& j, const wolf_sound_variant_t::sections_t& val) {
            std::unordered_map<std::string, sound_event_t> entries;
            if (val.m_ambient_sound.has_value()) entries.insert({ "ambient_sound", val.m_ambient_sound.value() });
            if (val.m_death_sound.has_value()) entries.insert({ "death_sound", val.m_death_sound.value() });
            if (val.m_hurt_sound.has_value()) entries.insert({ "hurt_sound", val.m_hurt_sound.value() });
            if (val.m_growl_sound.has_value()) entries.insert({ "growl_sound", val.m_growl_sound.value() });
            if (val.m_pant_sound.has_value()) entries.insert({ "pant_sound", val.m_pant_sound.value() });
            if (val.m_whine_sound.has_value()) entries.insert({ "whine_sound", val.m_whine_sound.value() });
            if (val.m_step_sound.has_value()) entries.insert({ "step_sound", val.m_step_sound.value() });
            j = entries;
        }
        friend void from_json(const nlohmann::json& j, wolf_sound_variant_t::sections_t& val) {
            if (j.contains("ambient_sound")) val.m_ambient_sound = sound_event_t(j["ambient_sound"]);
            if (j.contains("death_sound")) val.m_death_sound = sound_event_t(j["death_sound"]);
            if (j.contains("hurt_sound")) val.m_hurt_sound = sound_event_t(j["hurt_sound"]);
            if (j.contains("growl_sound")) val.m_growl_sound = sound_event_t(j["growl_sound"]);
            if (j.contains("pant_sound")) val.m_pant_sound = sound_event_t(j["pant_sound"]);
            if (j.contains("whine_sound")) val.m_whine_sound = sound_event_t(j["whine_sound"]);
            if (j.contains("step_sound")) val.m_step_sound = sound_event_t(j["step_sound"]);
        }
    };

    sections_t adult_sounds;
    sections_t baby_sounds;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(cat_variant_t, asset_id, baby_asset_id, spawn_conditions)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(frog_variant_t, asset_id, spawn_conditions)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(wolf_variant_t::sections_t, angry, wild, tame)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(wolf_variant_t, assets, baby_assets, spawn_conditions)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(cat_sound_variant_t, adult_sounds, baby_sounds)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(chicken_sound_variant_t, adult_sounds, baby_sounds)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(pig_sound_variant_t, adult_sounds, baby_sounds)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(wolf_sound_variant_t, adult_sounds, baby_sounds)

}
