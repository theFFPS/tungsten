/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/type/text_component.hpp>
#include <tungsten/util/json_reflect.hpp>
#include <tungsten/resource/holder.hpp>
#include <tungsten/world/block/banner_pattern.hpp>
#include <tungsten/world/item/item_stack.hpp>
#include <tungsten/world/block/trial_spawner.hpp>
#include <tungsten/world/entity/entity.hpp>

namespace tungsten {

struct data_components_t;

struct block_entity__banner_t {
    struct pattern_t {
        std::string color;
        holder_t<banner_pattern_t> pattern;
    };

    static constexpr std::string_view type_tag = "banner"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::vector<pattern_t> patterns;
}; // extra: has pattern_t
struct block_entity__barrel_t {
    static constexpr std::string_view type_tag = "barrel"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::optional<item_stack_t> lock;
    std::optional<resource_location_t> LootTable;
    std::optional<std::int64_t> LootTableSeed;
    std::vector<item_stack_t> Items;
};
struct block_entity__beacon_t {
    static constexpr std::string_view type_tag = "beacon"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::optional<item_stack_t> lock;
    std::optional<std::string> primary_effect;
    std::optional<std::string> secondary_effect;
};
struct block_entity__beehive_t {
    static constexpr std::string_view type_tag = "beehive"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;

    struct bee_t {
        generic_entity_format_t entity_data;
        std::int32_t min_ticks_in_hive;
        std::int32_t ticks_in_hive;
    };

    std::vector<bee_t> bees;
    std::array<std::int32_t, 3> flower_pos;
};
struct block_entity__bell_t {
    static constexpr std::string_view type_tag = "bell"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
};
struct block_entity__blast_furnace_t {
    static constexpr std::string_view type_tag = "blast_furnace"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::optional<item_stack_t> lock;
    std::vector<item_stack_t> Items;
    short lit_time_remaining;
    short cooking_time_spent;
    short cooking_total_time;
    short lit_total_time;
    std::unordered_map<std::string, std::int32_t> RecipesUsed;
};
struct block_entity__brewing_stand_t {
    static constexpr std::string_view type_tag = "brewing_stand"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::optional<item_stack_t> lock;
    std::vector<item_stack_t> Items;
    short BrewTIme;
    std::optional<std::int8_t> Fuel;
};
struct block_entity__brushable_block_t {
    static constexpr std::string_view type_tag = "brushable_block"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<resource_location_t> LootTable;
    std::optional<std::int64_t> LootTableSeed;
    std::optional<item_stack_t> item;
};
struct block_entity__campfire_t {
    static constexpr std::string_view type_tag = "campfire"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::vector<item_stack_t> Items;
    std::vector<int> CookingTimes;
    std::vector<int> CookingTotalTimes;
};
struct block_entity__chiseled_bookshelf_t {
    static constexpr std::string_view type_tag = "chiseled_bookshelf"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::vector<item_stack_t> Items;
    std::int32_t last_interacted_slot = -1;
};
struct block_entity__chest_t {
    static constexpr std::string_view type_tag = "chest"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::optional<item_stack_t> lock;
    std::optional<resource_location_t> LootTable;
    std::optional<std::int64_t> LootTableSeed;
    std::vector<item_stack_t> Items;
};
struct block_entity__comparator_t {
    static constexpr std::string_view type_tag = "comparator"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::int32_t OutputSignal;
};
struct block_entity__command_block_t {
    static constexpr std::string_view type_tag = "command_block"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    bool auto_;
    std::string Command;
    std::optional<bool> conditionMet;
    std::optional<std::int64_t> LastExecution;
    std::optional<std::string> LastOutput;
    std::optional<bool> powered;
    std::optional<int> SuccessCount;
    std::optional<bool> TrackOutput;
    bool UpdateLastExecution = true;
};
struct block_entity__conduit_t {
    static constexpr std::string_view type_tag = "conduit"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<std::array<std::int32_t, 4>> Target;
};
struct block_entity__copper_golem_statue_t {
    static constexpr std::string_view type_tag = "copper_golem_statue"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
};
struct block_entity__crafter_t {
    static constexpr std::string_view type_tag = "crafter"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::optional<item_stack_t> lock;
    std::optional<resource_location_t> LootTable;
    std::optional<std::int64_t> LootTableSeed;
    std::vector<item_stack_t> Items;
    std::vector<int> disabled_slots;
    bool triggered;
    std::int32_t crafting_ticks_remaining;
};
struct block_entity__creaking_heart_t {
    static constexpr std::string_view type_tag = "creaking_heart"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<std::array<std::int32_t, 4>> creaking;
};
struct block_entity__daylight_detector_t {
    static constexpr std::string_view type_tag = "daylight_detector"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
};
struct block_entity__decorated_pot_t {
    struct sherds_t {
        std::optional<item_stack_t> left;
        std::optional<item_stack_t> back;
        std::optional<item_stack_t> right;
        std::optional<item_stack_t> front;
    };

    static constexpr std::string_view type_tag = "decorated_pot"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<resource_location_t> LootTable;
    std::optional<std::int64_t> LootTableSeed;
    std::optional<item_stack_t> item;
    std::optional<sherds_t> sherds;
};
struct block_entity__dispenser_t {
    static constexpr std::string_view type_tag = "dispenser"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::optional<item_stack_t> lock;
    std::optional<resource_location_t> LootTable;
    std::optional<std::int64_t> LootTableSeed;
    std::vector<item_stack_t> Items;
};
struct block_entity__dropper_t {
    static constexpr std::string_view type_tag = "dropper"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::optional<item_stack_t> lock;
    std::optional<resource_location_t> LootTable;
    std::optional<std::int64_t> LootTableSeed;
    std::vector<item_stack_t> Items;
    std::optional<bool> Lunar;
};
struct block_entity__enchanting_table_t {
    static constexpr std::string_view type_tag = "enchanting_table"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
};
struct block_entity__ender_chest_t {
    static constexpr std::string_view type_tag = "ender_chest"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
};
struct block_entity__end_gateway_t {
    static constexpr std::string_view type_tag = "end_gateway"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::int64_t Age;
    bool ExactTeleport;
    std::optional<std::array<int, 3>> exit_portal;
};
struct block_entity__end_portal_t {
    static constexpr std::string_view type_tag = "end_portal"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
};
struct block_entity__furnace_t {
    static constexpr std::string_view type_tag = "furnace"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::optional<item_stack_t> lock;
    std::vector<item_stack_t> Items;
    short lit_time_remaining;
    short cooking_time_spent;
    short cooking_total_time;
    short lit_total_time;
    std::unordered_map<std::string, std::int32_t> RecipesUsed;
};
struct block_entity__hopper_t {
    static constexpr std::string_view type_tag = "hopper"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::optional<item_stack_t> lock;
    std::optional<resource_location_t> LootTable;
    std::optional<std::int64_t> LootTableSeed;
    std::vector<item_stack_t> Items;
    std::int32_t TransferCooldown = 0;
};
struct block_entity__jigsaw_t {
    static constexpr std::string_view type_tag = "jigsaw"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::string final_state;
    std::string joint; // rollable || aligned
    std::string name;
    std::string pool;
    std::string target;
    std::int32_t selection_priority;
    std::int32_t placement_priority;
};
struct block_entity__jukebox_t {
    static constexpr std::string_view type_tag = "jukebox"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<item_stack_t> RecordItem;
    std::optional<std::int64_t> ticks_since_song_started;
};
struct block_entity__lectern_t {
    static constexpr std::string_view type_tag = "lectern"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<item_stack_t> Book;
    std::optional<int> Page;
};
struct block_entity__mob_spawner_t {
    static constexpr std::string_view type_tag = "mob_spawner"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    short Delay;
    short MaxNearbyEntities;
    short MaxSpawnDelay;
    short MinSpawnDelay;
    short RequiredPlayerRange;
    short SpawnCount;
    std::optional<spawn_data_t> SpawnData;
    std::optional<std::vector<spawn_potential_t>> SpawnPotentials;
    short SpawnRange;
};
struct block_entity__piston_t {
    static constexpr std::string_view type_tag = "piston"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    block_state_t blockState;
    std::int8_t extending;
    std::int32_t facing; // 0=down, 1=up, 2=north, 3=south, 4=west, 5=east
    float progress;
    std::int8_t source;
};
struct block_entity__potent_sulfur_t {
    static constexpr std::string_view type_tag = "potent_sulfur"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::int32_t countdown;
};
struct block_entity__sculk_catalyst_t {
    struct cursor_t {
        std::int32_t charge;
        std::array<int, 3> pos;
        std::int32_t decay_delay;
        std::int32_t update_delay;
        std::vector<std::string> facings;
    };

    static constexpr std::string_view type_tag = "sculk_catalyst"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::vector<cursor_t> cursors;
};
struct block_entity__sculk_sensor_t {
    static constexpr std::string_view type_tag = "sculk_sensor"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::int32_t last_vibration_frequency;
    struct listener_t {
        struct event_t {
            float distance;
            resource_location_t game_event;
            std::array<double, 3> pos;
            std::optional<std::array<std::int32_t, 4>> projectile_owner;
            std::optional<std::array<std::int32_t, 4>> source;
        } event;
        std::int32_t event_delay = 0;
        struct selector_t {
            std::int64_t tick = -1;
            event_t event;
        } selector;
    } listener;
};
struct block_entity__calibrated_sculk_sensor_t {
    static constexpr std::string_view type_tag = "calibrated_sculk_sensor"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::int32_t last_vibration_frequency;
    block_entity__sculk_sensor_t::listener_t listener;
};
struct block_entity__sculk_shrieker_t {
    static constexpr std::string_view type_tag = "sculk_shrieker"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
};
struct block_entity__shulker_box_t {
    static constexpr std::string_view type_tag = "shulker_box"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::optional<item_stack_t> lock;
    std::optional<resource_location_t> LootTable;
    std::optional<std::int64_t> LootTableSeed;
    std::vector<item_stack_t> Items;
};
struct block_entity__sign_t {
    struct sign_text_t {
        std::vector<text_t> messages;
        std::optional<std::vector<text_t>> filtered_messages;
        std::string color = "black";
        bool has_glowing_text;
    };

    static constexpr std::string_view type_tag = "sign"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    bool is_waxed = false;
    bool allow_op_features = false;
    sign_text_t front_text;
    sign_text_t back_text;
}; // extra: has sign text field
struct block_entity__hanging_sign_t {
    static constexpr std::string_view type_tag = "hanging_sign"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    bool is_waxed = false;
    bool allow_op_features = false;
    block_entity__sign_t::sign_text_t front_text;
    block_entity__sign_t::sign_text_t back_text;
};
struct block_entity__skull_t {
    static constexpr std::string_view type_tag = "skull"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<std::string> custom_name;
    std::optional<std::string> note_block_sound;
    std::optional<std::variant<std::string, game_profile_t>> profile;
};
struct block_entity__smoker_t {
    static constexpr std::string_view type_tag = "smoker"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::optional<item_stack_t> lock;
    std::vector<item_stack_t> Items;
    short lit_time_remaining;
    short cooking_time_spent;
    short cooking_total_time;
    short lit_total_time;
    std::unordered_map<std::string, std::int32_t> RecipesUsed;
};
struct block_entity__soul_campfire_t {
    static constexpr std::string_view type_tag = "soul_campfire"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::vector<item_stack_t> Items;
    std::vector<int> CookingTimes;
    std::vector<int> CookingTotalTimes;
};
struct block_entity__structure_block_t {
    static constexpr std::string_view type_tag = "structure_block"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::string author;
    bool ignoreEntities;
    float integrity;
    std::string metadata;
    std::string mirror; // NONE || LEFT_RIGHT || FRONT_BACK
    std::string mode; // SAVE || LOAD || CORNER || DATA
    std::string name;
    std::int32_t posX;
    std::int32_t posY;
    std::int32_t posZ;
    bool powered;
    std::string rotation; // NONE || CLOCKWISE_90 || CLOCKWISE_180 || COUNTERCLOCKWISE_90
    std::int64_t seed;
    bool showboundingbox;
    std::int32_t sizeX;
    std::int32_t sizeY;
    std::int32_t sizeZ;
};
struct block_entity__test_block_t {
    static constexpr std::string_view type_tag = "test_block"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
};
struct block_entity__test_instance_block_t {
    static constexpr std::string_view type_tag = "test_instance_block"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
};
struct block_entity__trapped_chest_t {
    static constexpr std::string_view type_tag = "trapped_chest"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::optional<text_t> CustomName;
    std::optional<item_stack_t> lock;
    std::optional<resource_location_t> LootTable;
    std::optional<std::int64_t> LootTableSeed;
    std::vector<item_stack_t> Items;
};
struct block_entity__trial_spawner_t {
    static constexpr std::string_view type_tag = "trial_spawner"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    std::int32_t required_player_range;
    std::int32_t target_cooldown_length = 36000;
    holder_t<trial_spawner_t> normal_config;
    holder_t<trial_spawner_t> ominous_config;
    std::vector<std::array<std::int32_t, 4>> registered_players;
    std::vector<std::array<std::int32_t, 4>> current_mobs;
    std::int64_t cooldown_ends_at;
    std::int64_t next_mob_spawns_at;
    std::int32_t total_mobs_spawned;
    spawn_data_t spawn_data;
    resource_location_t ejecting_loot_table;
};
struct block_entity__vault_t {
    static constexpr std::string_view type_tag = "vault"; static constexpr std::string_view type_key = "id";
    bool keepPacked;
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::optional<std::shared_ptr<data_components_t>> components;
    struct config_t {
        resource_location_t loot_table;
        resource_location_t override_loot_table_to_display;
        double activation_range = 4;
        double deactivation_range = 4.5;
        item_stack_t key_item;
    } config;
    struct server_data_t {
        std::vector<std::array<std::int32_t, 4>> rewarded_players;
        std::int64_t state_updating_resumes_at;
        std::vector<item_stack_t> items_to_eject;
        std::int32_t total_ejections_needed;
    } server_data;
    struct shared_data_t {
        item_stack_t display_item;
        std::vector<std::array<std::int32_t, 4>> connected_players;
        double connected_particles_range;
    } shared_data;
};

using block_entity_t = std::variant<block_entity__banner_t, block_entity__barrel_t, block_entity__beacon_t, block_entity__beehive_t, block_entity__bell_t,
    block_entity__blast_furnace_t, block_entity__brewing_stand_t, block_entity__brushable_block_t, block_entity__calibrated_sculk_sensor_t,
    block_entity__campfire_t, block_entity__chiseled_bookshelf_t, block_entity__chest_t, block_entity__comparator_t, block_entity__command_block_t,
    block_entity__conduit_t, block_entity__copper_golem_statue_t, block_entity__crafter_t, block_entity__creaking_heart_t, block_entity__daylight_detector_t,
    block_entity__decorated_pot_t, block_entity__dispenser_t, block_entity__dropper_t, block_entity__enchanting_table_t, block_entity__ender_chest_t,
    block_entity__end_gateway_t, block_entity__end_portal_t, block_entity__furnace_t, block_entity__hanging_sign_t, block_entity__hopper_t,
    block_entity__jigsaw_t, block_entity__jukebox_t, block_entity__lectern_t, block_entity__mob_spawner_t, block_entity__piston_t, block_entity__potent_sulfur_t,
    block_entity__sculk_catalyst_t, block_entity__sculk_sensor_t, block_entity__sculk_shrieker_t, block_entity__shulker_box_t, block_entity__sign_t,
    block_entity__skull_t, block_entity__smoker_t, block_entity__soul_campfire_t, block_entity__structure_block_t, block_entity__test_block_t,
    block_entity__test_instance_block_t, block_entity__trapped_chest_t, block_entity__trial_spawner_t, block_entity__vault_t>;
struct wrapped_block_entity_t {
    block_entity_t value;

    struct glaze {
        using R = wrapped_block_entity_t;
        static constexpr auto value = &R::value;
    };
};

}

// members named like a C++ keyword carry a trailing underscore which is not part of the key
template <> struct glz::meta<tungsten::block_entity__command_block_t> {
    static constexpr std::string_view rename_key(const std::string_view key) { return key.ends_with('_') ? key.substr(0, key.size() - 1) : key; }
};
