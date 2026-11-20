/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/resource/resource_location.hpp>
#include <tungsten/type/text_component.hpp>
#include <tungsten/type/sound_event.hpp>
#include <tungsten/util/json_reflect.hpp>
#include <map>
#include <variant>

namespace tungsten {

struct environment_attribute_t {
    struct gameplay_bed_rule_t {
        std::string can_sleep; // always || when_dark || never
        std::string can_set_spawn; // always || when_dark || never
        bool explodes = false;
        bool destroy_on_use = false;
        std::optional<text_t> error_message;
    };
    struct audio_ambient_sounds_t {
        struct mood_t {
            int block_search_extent;
            int offset;
            sound_event_t sound;
            int tick_delay;
        };
        struct additions_t {
            sound_event_t sound;
            float tick_chance;
        };

        std::optional<mood_t> mood;
        std::optional<additions_t> additions;
        std::optional<sound_event_t> loop;
    };
    struct audio_background_music_t {
        struct entry_t {
            int max_delay;
            int min_delay;
            sound_event_t sound;
            bool replace_current_music = false;
        };

        std::optional<entry_t> default_;
        std::optional<entry_t> creative;
        std::optional<entry_t> underwater;
    };

    resource_location_t type;
    std::variant<bool, int, float, std::string, gameplay_bed_rule_t, audio_ambient_sounds_t, audio_background_music_t> value;

    bool is_boolean() {
        return type == resource_location_t("audio/firefly_bush_sounds") || type == resource_location_t("gameplay/bees_stay_in_hive") ||
            type == resource_location_t("gameplay/can_pillager_patrol_spawn") || type == resource_location_t("gameplay/creaking_active") ||
            type == resource_location_t("gameplay/can_start_raid") || type == resource_location_t("gameplay/eyeblossom_open") ||
            type == resource_location_t("gameplay/fast_lava") || type == resource_location_t("gameplay/increased_fire_burnout") ||
            type == resource_location_t("gameplay/monsters_burn") || type == resource_location_t("gameplay/nether_portal_spawns_piglin") ||
            type == resource_location_t("gameplay/piglins_zombify") || type == resource_location_t("gameplay/respawn_anchor_works") ||
            type == resource_location_t("gameplay/snow_golem_melts") || type == resource_location_t("gameplay/water_evaporates") ||
            type == resource_location_t("visual/has_sky_occluder");
    }
    bool is_string() {
        return type == resource_location_t("gameplay/baby_villager_activity") || type == resource_location_t("gameplay/eyeblossom_open") ||
            type == resource_location_t("visual/moon_phase") || type == resource_location_t("gameplay/villager_activity");
    }
    bool is_float() {
        return type == resource_location_t("gameplay/sky_light_level") || type == resource_location_t("gameplay/surface_slime_spawn_chance") ||
            type == resource_location_t("gameplay/turtle_egg_hatch_chance") || type == resource_location_t("visual/cloud_fog_end_distance") ||
            type == resource_location_t("visual/fog_end_distance") || type == resource_location_t("visual/cloud_height") ||
            type == resource_location_t("visual/fog_start_distance") || type == resource_location_t("visual/moon_angle") ||
            type == resource_location_t("visual/sky_light_factor") || type == resource_location_t("visual/sky_fog_end_distance") ||
            type == resource_location_t("visual/star_brightness") || type == resource_location_t("visual/star_angle") ||
            type == resource_location_t("visual/water_fog_end_distance") || type == resource_location_t("visual/sun_angle") ||
            type == resource_location_t("visual/water_fog_start_distance") || type == resource_location_t("gameplay/cat_waking_up_gift_chance") ||
            type == resource_location_t("audio/music_volume");
    }

    // `type` is the attribute id (the key of the map this lives in) and has to be set before reading; the shape depends on it
    void from_generic(const glz::generic& j) {
        using jr::from_generic;
        if (j.is_boolean() && is_boolean()) value = j.get_boolean();
        else if (j.is_string() && is_string()) value = j.get_string();
        else if (j.is_number() && is_float()) value = static_cast<float>(j.get_number());
        else if (j.is_object() && type == resource_location_t("gameplay/bed_rule")) value = from_generic<gameplay_bed_rule_t>(j);
        else if (j.is_object() && type == resource_location_t("audio/background_music")) value = from_generic<audio_background_music_t>(j);
        else if (j.is_object() && type == resource_location_t("audio/ambient_sounds")) value = from_generic<audio_ambient_sounds_t>(j);
    }
    glz::generic to_generic() const {
        return std::visit([](const auto& v) -> glz::generic {
            using V = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<V, bool> || std::is_same_v<V, int> || std::is_same_v<V, float> || std::is_same_v<V, std::string>) return jr::to_generic(v);
            else return jr::to_generic(v);
        }, value);
    }
};
struct environment_attribute_map_t {
    std::map<resource_location_t, environment_attribute_t> value;

    bool empty() const noexcept { return value.empty(); }
    bool contains(const resource_location_t& id) const { return value.contains(id); }
    const environment_attribute_t* find(const resource_location_t& id) const {
        auto it = value.find(id);
        return it == value.end() ? nullptr : &it->second;
    }

    // { "<attribute id>": <value> }
    void from_generic(const glz::generic& j) {
        *this = environment_attribute_map_t();
        if (!j.is_object()) throw jr::error("", "expected an object");
        for (const auto& [key, element] : j.get_object()) {
            try {
                environment_attribute_t attribute;
                attribute.type = resource_location_t(key);
                attribute.from_generic(element);
                value.emplace(attribute.type, std::move(attribute));
            } catch (const jr::error& e) {
                throw jr::error("[" + key + "]", e.m_message);
            } catch (const std::exception& e) {
                throw jr::error("[" + key + "]", e.what());
            }
        }
    }
    glz::generic to_generic() const {
        glz::generic j = glz::generic::object_t{};
        for (const auto& [id, attribute] : value) j[id.to_string()] = attribute.to_generic();
        return j;
    }

    struct glaze {
        using R = environment_attribute_map_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

}

// the key of `default_` is "default"
template <> struct glz::meta<tungsten::environment_attribute_t::audio_background_music_t> {
    static constexpr std::string_view rename_key(const std::string_view key) { return key.ends_with('_') ? key.substr(0, key.size() - 1) : key; }
};
