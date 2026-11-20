/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <vector>
#include <optional>
#include <tungsten/resource/resource_location.hpp>
#include <tungsten/type/variant.hpp>
#include <tungsten/type/text_component.hpp>
#include <tungsten/type/int_provider.hpp>
#include <tungsten/resource/holder_set.hpp>
#include <tungsten/util/json_range.hpp>
#include <tungsten/world/block/block_entity.hpp>
#include "data_component.hpp"
#include "level_based_value.hpp"

namespace tungsten {

struct wrapped_context_number_provider_t;
struct wrapped_wrapped_context_number_provider_t {
    std::shared_ptr<wrapped_context_number_provider_t> value;

    // serialized as the provider itself (defined in predicate.cpp where the provider is complete)
    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = wrapped_wrapped_context_number_provider_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};
struct item_predicate_t;
struct wrapped_item_predicate_t {
    std::shared_ptr<item_predicate_t> value;

    // serialized as the predicate itself (defined in predicate.cpp where the predicate is complete)
    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = wrapped_item_predicate_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

struct effect_predicate_t {
    std::optional<int_range_t> amplifier;
    std::optional<int_range_t> duration;
    std::optional<bool> ambient;
    std::optional<bool> visible;
};
struct firework_explosion_effect_predicate_t {
    std::optional<std::string> shape; // small_ball || large_ball || star || burst || creeper
    std::optional<bool> has_trail;
    std::optional<bool> has_twinkle;
};
struct attribute_modifier_predicate_t {
    std::optional<holder_set_t> attribute;
    std::optional<resource_location_t> id;
    std::optional<double_range_t> amount;
    std::optional<std::string> operation; // add_value || add_multiplied_base || add_multiplied_total
    std::optional<std::string> slot; // any || mainhand || offhand || hand || head || chest || legs || feet || armor || body
};
struct data_component_predicate_t {
    using ranged_int_t = int_range_t;

    struct attribute_modifiers_t {
        using modifiers_t = collection_predicate_t<attribute_modifier_predicate_t>;

        std::optional<modifiers_t> modifiers;
    };
    struct bundle_contents_t {
        using items_t = collection_predicate_t<wrapped_item_predicate_t>;

        std::optional<items_t> items;
    };
    struct container_t {
        using items_t = collection_predicate_t<wrapped_item_predicate_t>;

        std::optional<items_t> items;
    };
    using custom_data_t = std::variant<std::string, glz::generic>;
    struct damage_t {
        std::optional<ranged_int_t> damage;
        std::optional<ranged_int_t> durability;
    };
    struct enchantment_t {
        std::optional<holder_set_t> enchantments;
        std::optional<ranged_int_t> levels;
    };
    using firework_explosion_t = firework_explosion_effect_predicate_t;
    struct fireworks_t {
        using explosions_t = collection_predicate_t<firework_explosion_effect_predicate_t>;

        std::optional<explosions_t> explosions;
        std::optional<ranged_int_t> flight_duration;
    };
    struct jukebox_playable_t { std::optional<holder_set_t> song; };
    struct potion_contents_t {
        using effects_t = collection_predicate_t<std::unordered_map<std::string, effect_predicate_t>>;

        std::optional<holder_set_t> potions;
        std::optional<effects_t> effects;
    };
    struct trim_t {
        std::optional<holder_set_t> material;
        std::optional<holder_set_t> pattern;
    };
    struct writable_book_content_t {
        using pages_t = collection_predicate_t<std::string>;

        std::optional<pages_t> pages;
    };
    struct written_book_content_t {
        using pages_t = collection_predicate_t<text_t>;

        std::optional<pages_t> pages;
        std::optional<std::string> author;
        std::optional<std::string> title;
        std::optional<ranged_int_t> generation;
        std::optional<bool> resolved;
    };

    resource_location_t type;
    std::variant<attribute_modifiers_t, bundle_contents_t, container_t, custom_data_t, damage_t, std::vector<enchantment_t>, firework_explosion_t,
        fireworks_t, jukebox_playable_t, potion_contents_t, trim_t, writable_book_content_t, written_book_content_t> value;

    // `type` is the component id (the key the predicate is stored under) and has to be set before reading
    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;
};
struct data_component_predicates_t {
    std::map<resource_location_t, data_component_predicate_t> value;
    std::vector<resource_location_t> removed;

    bool empty() const noexcept { return value.empty() && removed.empty(); }
    bool contains(const resource_location_t& id) const { return value.contains(id); }
    const data_component_predicate_t* find(const resource_location_t& id) const {
        auto it = value.find(id);
        return it == value.end() ? nullptr : &it->second;
    }

    // { "<id>": <predicate>, "!<id>": {} } where the "!" form requires the component to be absent
    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = data_component_predicates_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

struct item_predicate_t {
    std::optional<holder_set_t> items;
    std::optional<int_range_t> count;
    std::optional<data_components_t> components;
    std::optional<data_component_predicates_t> predicates;
};
struct location_predicate_t {
    struct block_predicate_t {
        std::optional<holder_set_t> blocks;
        std::optional<std::unordered_map<std::string, std::variant<std::string, range_t<std::string>>>> state;
        std::optional<std::variant<std::string, glz::generic>> nbt;
        std::optional<data_components_t> components;
        std::optional<data_component_predicates_t> predicates;
    };
    struct fluid_predicate_t {
        std::optional<holder_set_t> fluids;
        std::optional<std::unordered_map<std::string, std::variant<std::string, range_t<std::string>>>> state;
    };
    struct light_predicate_t {
        std::optional<int_range_t> light;
    };
    struct position_predicate_t {
        std::optional<double_range_t> x;
        std::optional<double_range_t> y;
        std::optional<double_range_t> z;
    };

    std::optional<holder_set_t> biomes;
    std::optional<block_predicate_t> block;
    std::optional<resource_location_t> dimension;
    std::optional<fluid_predicate_t> fluid;
    std::optional<light_predicate_t> light;
    std::optional<position_predicate_t> position;
    std::optional<bool> smokey;
    std::optional<bool> can_see_sky;
    std::optional<holder_set_t> structures;
};
struct entity_predicate_t {
    struct distance_t {
        std::optional<float_range_t> absolute;
        std::optional<float_range_t> horizontal;
        std::optional<float_range_t> x;
        std::optional<float_range_t> y;
        std::optional<float_range_t> z;
    };
    using effects_t = std::unordered_map<std::string, effect_predicate_t>;

    struct entity_tags_t {
        std::optional<std::vector<std::string>> any_of;
        std::optional<std::vector<std::string>> all_of;
        std::optional<std::vector<std::string>> none_of;
    };
    struct equipment_t {
        std::optional<item_predicate_t> mainhand;
        std::optional<item_predicate_t> offhand;
        std::optional<item_predicate_t> head;
        std::optional<item_predicate_t> chest;
        std::optional<item_predicate_t> legs;
        std::optional<item_predicate_t> feet;
        std::optional<item_predicate_t> body;
    };
    struct flags_t {
        std::optional<bool> is_baby;
        std::optional<bool> is_on_fire;
        std::optional<bool> is_sneaking;
        std::optional<bool> is_sprinting;
        std::optional<bool> is_swimming;
        std::optional<bool> is_on_ground;
        std::optional<bool> is_flying;
        std::optional<bool> is_fall_flying;
    };
    struct movement_t {
        std::optional<double_range_t> x;
        std::optional<double_range_t> y;
        std::optional<double_range_t> z;
        std::optional<double_range_t> speed;
        std::optional<double_range_t> horizontal_speed;
        std::optional<double_range_t> vertical_speed;
        std::optional<double_range_t> fall_distance;
    };
    struct type_specific__fishing_hook_t {
        std::optional<bool> is_open_water;
    };
    struct type_specific__lightning_t {
        std::optional<int_range_t> blocks_set_on_fire;
        std::optional<std::shared_ptr<entity_predicate_t>> entity_struck;
    };
    struct type_specific__player_t {
        using advancement_predicate_t = std::variant<bool, std::unordered_map<std::string, bool>>;
        struct stat_t {
            std::optional<resource_location_t> type; // custom || crafted || used || broken || mined || killed_by || killed || picked_up || dropped
            std::optional<std::string> stat;
            std::optional<int_range_t> value;
        };
        struct input_t {
            std::optional<bool> forward;
            std::optional<bool> backward;
            std::optional<bool> left;
            std::optional<bool> right;
            std::optional<bool> jump;
            std::optional<bool> sneak;
            std::optional<bool> sprint;
        };
        struct food_t {
            std::optional<int_range_t> level;
            std::optional<float_range_t> saturation;
        };

        std::optional<std::shared_ptr<entity_predicate_t>> looking_at;
        std::optional<std::unordered_map<std::string, advancement_predicate_t>> advancements;
        std::optional<std::string> gamemode;
        std::optional<int_range_t> level;
        std::optional<std::unordered_map<std::string, bool>> recipes;
        std::optional<std::vector<stat_t>> stats;
        std::optional<input_t> input;
        std::optional<food_t> food;
    };
    struct type_specific__raider_t {
        std::optional<bool> is_captain;
        std::optional<bool> has_raid;
    };
    struct type_specific__sheep_t {
        std::optional<bool> sheared;
    };
    struct type_specific__cube_mob_t {
        std::optional<int_range_t> size;
    };

    std::optional<holder_set_t> entity_type;
    std::optional<data_components_t> components;
    std::optional<distance_t> distance;
    std::optional<effects_t> effects;
    std::optional<entity_tags_t> entity_tags;
    std::optional<equipment_t> equipment;
    std::optional<flags_t> flags;
    std::optional<location_predicate_t> location;
    std::optional<std::variant<std::string, glz::generic>> nbt;
    std::optional<std::shared_ptr<entity_predicate_t>> passenger;
    std::optional<std::unordered_map<std::string, item_predicate_t>> slots;
    std::optional<location_predicate_t> stepping_on;
    std::optional<location_predicate_t> movement_affected_by;
    std::optional<std::string> team;
    std::optional<std::shared_ptr<entity_predicate_t>> targeted_entity;
    std::optional<std::shared_ptr<entity_predicate_t>> vehicle;
    std::optional<movement_t> movement;
    std::optional<int> periodic_tick;
    std::optional<data_component_predicates_t> predicates;
    std::optional<type_specific__fishing_hook_t> type_specific__fishing_hook;
    std::optional<type_specific__lightning_t> type_specific__lightning;
    std::optional<type_specific__player_t> type_specific__player;
    std::optional<type_specific__raider_t> type_specific__raider;
    std::optional<type_specific__sheep_t> type_specific__sheep;
    std::optional<type_specific__cube_mob_t> type_specific__cube_mob;

    // a flat object whose keys are resource locations ("minecraft:entity_type", "type_specific/player", ...)
    void from_generic(const glz::generic& j) {
        *this = entity_predicate_t();
        if (!j.is_object()) throw jr::error("", "expected an object");
        jr::read_key(j, "entity_type", entity_type);
        jr::read_key(j, "components", components);
        jr::read_key(j, "distance", distance);
        jr::read_key(j, "effects", effects);
        jr::read_key(j, "entity_tags", entity_tags);
        jr::read_key(j, "equipment", equipment);
        jr::read_key(j, "flags", flags);
        jr::read_key(j, "location", location);
        jr::read_key(j, "stepping_on", stepping_on);
        jr::read_key(j, "movement_affected_by", movement_affected_by);
        jr::read_key(j, "slots", slots);
        jr::read_key(j, "team", team);
        jr::read_key(j, "periodic_tick", periodic_tick);
        jr::read_key(j, "movement", movement);
        jr::read_key(j, "predicates", predicates);
        jr::read_key(j, "passenger", passenger);
        jr::read_key(j, "targeted_entity", targeted_entity);
        jr::read_key(j, "vehicle", vehicle);
        jr::read_key(j, "type_specific/fishing_hook", type_specific__fishing_hook);
        jr::read_key(j, "type_specific/lightning", type_specific__lightning);
        jr::read_key(j, "type_specific/player", type_specific__player);
        jr::read_key(j, "type_specific/raider", type_specific__raider);
        jr::read_key(j, "type_specific/sheep", type_specific__sheep);
        jr::read_key(j, "type_specific/cube_mob", type_specific__cube_mob);
        jr::read_key(j, "nbt", nbt);
    }
    glz::generic to_generic() const {
        glz::generic j = glz::generic::object_t{};
        jr::write_key(j, "minecraft:entity_type", entity_type);
        jr::write_key(j, "minecraft:components", components);
        jr::write_key(j, "minecraft:distance", distance);
        jr::write_key(j, "minecraft:effects", effects);
        jr::write_key(j, "minecraft:entity_tags", entity_tags);
        jr::write_key(j, "minecraft:equipment", equipment);
        jr::write_key(j, "minecraft:flags", flags);
        jr::write_key(j, "minecraft:location", location);
        jr::write_key(j, "minecraft:stepping_on", stepping_on);
        jr::write_key(j, "minecraft:movement_affected_by", movement_affected_by);
        jr::write_key(j, "minecraft:slots", slots);
        jr::write_key(j, "minecraft:team", team);
        jr::write_key(j, "minecraft:periodic_tick", periodic_tick);
        jr::write_key(j, "minecraft:movement", movement);
        jr::write_key(j, "minecraft:predicates", predicates);
        jr::write_key(j, "minecraft:passenger", passenger);
        jr::write_key(j, "minecraft:targeted_entity", targeted_entity);
        jr::write_key(j, "minecraft:vehicle", vehicle);
        jr::write_key(j, "minecraft:type_specific/fishing_hook", type_specific__fishing_hook);
        jr::write_key(j, "minecraft:type_specific/lightning", type_specific__lightning);
        jr::write_key(j, "minecraft:type_specific/player", type_specific__player);
        jr::write_key(j, "minecraft:type_specific/raider", type_specific__raider);
        jr::write_key(j, "minecraft:type_specific/sheep", type_specific__sheep);
        jr::write_key(j, "minecraft:type_specific/cube_mob", type_specific__cube_mob);
        jr::write_key(j, "nbt", nbt);
        return j;
    }

    struct glaze {
        using R = entity_predicate_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};
struct damage_type_predicate_t {
    struct entry_t {
        holder_set_t id;
        bool expected;
    };

    std::optional<entity_predicate_t> direct_entity;
    std::optional<entity_predicate_t> source_entity;
    std::optional<bool> is_direct;
    std::optional<std::vector<entry_t>> tags;
};

struct wrapped_predicate_t;
using ranged_context_number_provider_t = range_t<std::shared_ptr<wrapped_wrapped_context_number_provider_t>>;
using predicate_reference_t = std::variant<std::shared_ptr<wrapped_predicate_t>, std::vector<wrapped_predicate_t>, holder_set_t>;

struct predicate__all_of_t {
    static constexpr std::string_view type_tag = "all_of";
    predicate_reference_t terms;
};
struct predicate__any_of_t {
    static constexpr std::string_view type_tag = "any_of";
    predicate_reference_t terms;
};
struct predicate__damage_source_properties_t {
    static constexpr std::string_view type_tag = "damage_source_properties";
    damage_type_predicate_t predicate;
};
struct predicate__enchantment_active_check_t {
    static constexpr std::string_view type_tag = "enchantment_active_check";
    bool active;
};
struct predicate__entity_properties_t {
    static constexpr std::string_view type_tag = "entity_properties";
    std::string entity; // this || attacker || direct_attacker || attacking_player || target_entity || interacting_entity
    entity_predicate_t predicate;
};
struct predicate__entity_scores_t {
    static constexpr std::string_view type_tag = "entity_scores";
    std::string entity; // this || attacker || direct_attacker || attacking_player || target_entity || interacting_entity
    std::unordered_map<std::string, ranged_context_number_provider_t> scores;
};
struct predicate__float_value_check_t {
    static constexpr std::string_view type_tag = "float_value_check";
    std::variant<resource_location_t, std::shared_ptr<wrapped_wrapped_context_number_provider_t>> value;
    ranged_context_number_provider_t test;
};
struct predicate__int_value_check_t {
    static constexpr std::string_view type_tag = "int_value_check";
    std::variant<resource_location_t, std::shared_ptr<wrapped_wrapped_context_number_provider_t>> value;
    ranged_context_number_provider_t test;
};
struct predicate__inverted_t {
    static constexpr std::string_view type_tag = "inverted";
    std::variant<resource_location_t, std::shared_ptr<wrapped_predicate_t>> term;
};
struct predicate__killed_by_player_t {
    static constexpr std::string_view type_tag = "killed_by_player";
};
struct predicate__location_check_t {
    static constexpr std::string_view type_tag = "location_check";
    std::optional<int> offsetX;
    std::optional<int> offsetY;
    std::optional<int> offsetZ;
    location_predicate_t predicate;
};
struct predicate__match_block_t {
    static constexpr std::string_view type_tag = "match_block";
    std::optional<holder_set_t> blocks;
    std::optional<std::unordered_map<std::string, std::string>> state;
    std::optional<block_entity_t> nbt;
    std::optional<data_components_t> components;
    std::optional<data_component_predicates_t> predicates;
};
struct predicate__match_tool_t {
    static constexpr std::string_view type_tag = "match_tool";
    item_predicate_t predicate;
};
struct predicate__random_chance_t {
    static constexpr std::string_view type_tag = "random_chance";
    std::variant<resource_location_t, std::shared_ptr<wrapped_wrapped_context_number_provider_t>> chance;
};
struct predicate__random_chance_with_enchanted_bonus_t {
    static constexpr std::string_view type_tag = "random_chance_with_enchanted_bonus";
    float unenchanted_chance;
    std::shared_ptr<level_based_value_t> enchanted_chance;
    resource_location_t enchantment;
};
struct predicate__survives_explosion_t {
    static constexpr std::string_view type_tag = "survives_explosion";
};
struct predicate__table_bonus_t {
    static constexpr std::string_view type_tag = "table_bonus";
    resource_location_t enchantment;
    std::vector<std::variant<resource_location_t, std::shared_ptr<wrapped_wrapped_context_number_provider_t>>> chances;
};
struct predicate__time_check_t {
    static constexpr std::string_view type_tag = "time_check";
    resource_location_t clock;
    ranged_context_number_provider_t value;
    std::optional<int> period;
};
struct predicate__weather_check_t {
    static constexpr std::string_view type_tag = "weather_check";
    std::optional<bool> raining;
    std::optional<bool> thundering;
};

using predicate_t = std::variant<predicate__all_of_t, predicate__any_of_t, predicate__damage_source_properties_t, predicate__enchantment_active_check_t,
    predicate__entity_properties_t, predicate__entity_scores_t, predicate__float_value_check_t, predicate__int_value_check_t, predicate__inverted_t,
    predicate__killed_by_player_t, predicate__location_check_t, predicate__match_block_t, predicate__match_tool_t, predicate__random_chance_t,
    predicate__random_chance_with_enchanted_bonus_t, predicate__survives_explosion_t, predicate__table_bonus_t, predicate__time_check_t, predicate__weather_check_t>;

struct wrapped_predicate_t {
    predicate_t value;

    struct glaze {
        using R = wrapped_predicate_t;
        static constexpr auto value = &R::value;
    };
};

/*
 * predicate context requirements
 *  all_of = any
 *  any_if = any
 *  damage_source_properties =
 *      fields =
 *          origin
 *          damage_source
 *  enchantment_active_check =
 *      fields =
 *          enchantment_active_status
 *  entity_properties = any
 *  entity_scores =
 *      fields =
 *          any entity
 *  float_value_check = any
 *  int_value_check = any
 *  inverted = any
 *  killed_by_player =
 *      fields =
 *          attacking_player
 *  location_check =
 *      fields =
 *          origin
 *  match_block = ?
 *  match_tool =
 *      fields =
 *          tool
 *  random_chance = any
 *  random_chance_with_enchanted_bonus =
 *      fields =
 *          attacker
 *  survives_explosion =
 *      fields =
 *          explosion_radius
 *  table_bonus =
 *      fields =
 *          tool
 *  time_check = any
 *  weather_check = any
 */

struct wrapped_block_predicate_t;

struct block_predicate__all_of_t {
    static constexpr std::string_view type_tag = "all_of";
    std::vector<wrapped_block_predicate_t> predicates;
};
struct block_predicate__any_of_t {
    static constexpr std::string_view type_tag = "any_of";
    std::vector<wrapped_block_predicate_t> predicates;
};
struct block_predicate__has_sturdy_face_t {
    static constexpr std::string_view type_tag = "has_sturdy_face";
    std::vector<std::int32_t> offset = {0,0,0};
    std::string direction;
};
struct block_predicate__height_range_t {
    static constexpr std::string_view type_tag = "height_range";
    std::optional<vertical_anchor_t> min_inclusive;
    std::optional<vertical_anchor_t> max_inclusive;
};
struct block_predicate__inside_world_bounds_t {
    static constexpr std::string_view type_tag = "inside_world_bounds";
    std::vector<std::int32_t> offset = {0,0,0};
};
struct block_predicate__matching_biomes_t {
    static constexpr std::string_view type_tag = "matching_biomes";
    holder_set_t biomes;
};
struct block_predicate__matching_block_tag_t {
    static constexpr std::string_view type_tag = "matching_block_tag";
    std::vector<std::int32_t> offset = {0,0,0};
    std::string tag;
};
struct block_predicate__matching_blocks_t {
    static constexpr std::string_view type_tag = "matching_blocks";
    std::vector<std::int32_t> offset = {0,0,0};
    holder_set_t blocks;
};
struct block_predicate__matching_fluids_t {
    static constexpr std::string_view type_tag = "matching_fluids";
    std::vector<std::int32_t> offset = {0,0,0};
    holder_set_t fluids;
    std::optional<std::unordered_map<std::string, glz::generic>> state;
};
struct block_predicate__not_t {
    static constexpr std::string_view type_tag = "not";
    std::shared_ptr<wrapped_block_predicate_t> predicate;
};
struct block_predicate__replaceable_t {
    static constexpr std::string_view type_tag = "replaceable";
    std::vector<std::int32_t> offset = {0,0,0};
};
struct block_predicate__solid_t {
    static constexpr std::string_view type_tag = "solid";
    std::vector<std::int32_t> offset = {0,0,0};
};
struct block_predicate__true_t {
    static constexpr std::string_view type_tag = "true";
};
struct block_predicate__unobstructed_t {
    static constexpr std::string_view type_tag = "unobstructed";
    std::vector<std::int32_t> offset = {0,0,0};
};
struct block_predicate__volume_match_t {
    static constexpr std::string_view type_tag = "volume_match";
    std::vector<std::int32_t> min;
    std::vector<std::int32_t> max;
    std::shared_ptr<wrapped_block_predicate_t> match;
};
struct block_predicate__would_survive_t {
    static constexpr std::string_view type_tag = "would_survive";
    std::vector<std::int32_t> offset = {0,0,0};
    block_state_t state;
};

using block_predicate_t = std::variant<block_predicate__all_of_t, block_predicate__any_of_t, block_predicate__has_sturdy_face_t,
    block_predicate__height_range_t, block_predicate__inside_world_bounds_t, block_predicate__matching_biomes_t, block_predicate__matching_block_tag_t,
    block_predicate__matching_blocks_t, block_predicate__matching_fluids_t, block_predicate__not_t, block_predicate__replaceable_t,
    block_predicate__solid_t, block_predicate__true_t, block_predicate__unobstructed_t, block_predicate__volume_match_t, block_predicate__would_survive_t>;
struct wrapped_block_predicate_t {
    block_predicate_t value;

    struct glaze {
        using R = wrapped_block_predicate_t;
        static constexpr auto value = &R::value;
    };
};

}
