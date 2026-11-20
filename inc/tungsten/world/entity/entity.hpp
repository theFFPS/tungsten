/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/util/json_reflect.hpp>
#include <tungsten/type/text_component.hpp>
#include <tungsten/world/item/item_stack.hpp>
#include <tungsten/world/block/block_state.hpp>

namespace tungsten {

struct wrapped_block_entity_t;
struct wrapped_wrapped_block_entity_t {
    std::shared_ptr<wrapped_block_entity_t> value;

    // serialized as the block entity itself (defined in entity.cpp, where the block entity is complete)
    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = wrapped_wrapped_block_entity_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

struct spawn_data_t;
struct spawn_potential_t;

struct mob_effect_t { // VERIFY IF OK
    bool ambient;
    std::int8_t amplifier;
    std::int32_t duration;
    std::optional<std::shared_ptr<mob_effect_t>> hidden_effect;
    resource_location_t id;
    bool show_icon = true;
    bool show_particles;
};
struct mob_attribute_t {
    struct mob_attribute_modifier_t {
        double amount;
        resource_location_t id;
        std::string operation; // add_value || add_multiplied_base || add_multiplied_total
    };

    resource_location_t id;
    double base;
    std::vector<mob_attribute_modifier_t> modifiers;
};

struct wrapped_entity_format_t;
struct mob_brain_t {
    // TODO: memories — per-mob, variant-specific (allay, axolotl, camel, copper_golem, frog, goat, piglin, piglin_brute, sniffer, villager, warden)
    // field: memories
    //  each entry in it is a struct with tag being resource_location. think of it as environment attributes but just a bit different. u can implement it tho
    /*
     * [NBT Compound / JSON Object] memories: Used for complex behaviors.​[more information needed] The only mobs that have this tag are described below.

         Allay memories:[]
         [NBT Compound / JSON Object] minecraft:item_pickup_cooldown_ticks: The number of game ticks before the allay goes to pick up item again.
             [Int] value: The value of this memory, initially set to 100 game ticks (5 seconds), decreasing by 1 every tick.
         [NBT Compound / JSON Object] minecraft:liked_noteblock: The note block that the allay likes.
             [NBT Compound / JSON Object] value: The value of this memory.
                 [String] dimension: The ID of the dimension where the note block is.
                 [Int Array] pos: The X, Y, and Z coordinates of the note block.
         [NBT Compound / JSON Object] minecraft:liked_noteblock_cooldown_ticks: The number of ticks before the allay stops putting items at the liked note block.
             [Int] value: The value of this memory, initially set to 600 game ticks (30 seconds), decreasing by 1 every tick.
         [NBT Compound / JSON Object] minecraft:liked_player: The player that the allay likes.
             [Int Array] value: The player's UUID, stored as four ints.
         Axolotl memories:[]
         [NBT Compound / JSON Object] minecraft:has_hunting_cooldown: If the axolotl is in a hunting cooldown.[needs testing]
             [Boolean] value: 1 or 0 (true/false) - if true, the axolotl just hunted.
             [Long] ttl: The number of game ticks before this memory is removed.
         [NBT Compound / JSON Object] minecraft:play_dead_ticks: Optional. If the axolotl is pretending to be dead.
             [Int] value: The number of game ticks before the axolotl stops to play dead and this tag is removed.
         [NBT Compound / JSON Object] minecraft:is_tempted: If the axolotl is tempted by the player.
             [Boolean] value: 1 or 0 (true/false) - if true, the mob is tempted.
         [NBT Compound / JSON Object] minecraft:temptation_cooldown_ticks: Optional. If the axolotl is in a cooldown after being tempted.
             [Int] value: The number of game ticks before the mob can be tempted again and this tag is removed.
         Camel memories:[]
         [NBT Compound / JSON Object] minecraft:gaze_cooldown_ticks: Optional. If the camel is in a cooldown state after randomly looking around.
             [Int] value: The number of game ticks before the mob can randomly look around again and this tag is removed.
         [NBT Compound / JSON Object] minecraft:is_tempted: If the camel is tempted by the player.
             [Boolean] value: 1 or 0 (true/false) - if true, the mob is tempted.
         [NBT Compound / JSON Object] minecraft:temptation_cooldown_ticks: Optional. If the camel is in a cooldown after being tempted.
             [Int] value: The number of game ticks before the mob can be tempted again and this tag is removed.
         Copper Golem memories:[]
         [NBT Compound / JSON Object] minecraft:is_panicking: Optional. If the Copper Golem is panicking.
             [Boolean] value: 1 or 0 (true/false) - if true, the Copper Golem is panicking.
         [NBT Compound / JSON Object] minecraft:visited_block_positions: Saves the position of visited chests.
             [NBT List / JSON Array] value: List of visited chests.
                 [NBT Compound / JSON Object]: An individual modifier.
                     [Int Array] pos: The position of the chest.
                     [String] dimension: Dimensions of the chest.
             [Long] ttl: The number of game ticks before this memory is removed, initially set to 6000Lgame ticks (5 minutes).
         [NBT Compound / JSON Object] minecraft:gaze_cooldown_ticks: Optional. If the copper golem is in a cooldown state after randomly looking around.
             [Int] value: The number of game ticks before the mob can randomly look around again and this tag is removed.
         Frog memories:[]
         [NBT Compound / JSON Object] minecraft:is_in_water: Only exists if the frog is in water.
             [NBT Compound / JSON Object] value: An empty compound tag.
         [NBT Compound / JSON Object] minecraft:is_pregnant: Only exists if the frog is pregnant.
             [NBT Compound / JSON Object] value: An empty compound tag.
         Goat memories:[]
         [NBT Compound / JSON Object] minecraft:long_jump_cooling_down: Optional. If the goat is in a cool down after a long jump.
             [Int] value: The number of game ticks before the goat can long jump again and this tag is removed.
         [NBT Compound / JSON Object] minecraft:ram_cooldown_ticks: Optional. If the goat is in a cool down after ram.
             [Int] value: The number of game ticks before the goat can ram again and this tag is removed.
         [NBT Compound / JSON Object] minecraft:is_tempted: If the goat is tempted by the player.
             [Boolean] value: 1 or 0 (true/false) - if true, the mob is tempted.
         [NBT Compound / JSON Object] minecraft:temptation_cooldown_ticks: Optional. If the goat is in a cooldown after being tempted.
             [Int] value: The number of game ticks before the mob can be tempted again and this tag is removed.
         Piglin memories:[]
         [NBT Compound / JSON Object] minecraft:admiring_disabled: Piglins with this tag do not barter by right-clicking and are not distracted by gold items on the ground; set when converting, when attacked or when admiring an item.
             [Boolean] value: 1 or 0 (true/false) - if true, admiring is disabled.
             [Long] ttl: The number of game ticks before this memory is removed.
         [NBT Compound / JSON Object] minecraft:admiring_item: If the piglin is admiring an item.
             [Boolean] value: 1 or 0 (true/false) - if true, admiring an item.
             [Long] ttl: The number of game ticks before this memory is removed, the piglin throws back another item when this reaches 0, if it held a gold ingot.
         [NBT Compound / JSON Object] minecraft:angry_at: The target of this piglin. Set after a piglin or piglin brute is attacked.
             [Int Array] value: Universally Unique IDentifier of the entity that the piglin targets, stored as four ints.
             [Long] ttl: The number of game ticks before this memory is removed.
         [NBT Compound / JSON Object] minecraft:hunted_recently: If the piglin just hunted, and as such, don't hunt for a while. Set after hunting or spawning as part of a bastion remnant during world generation.
             [Boolean] value: 1 or 0 (true/false) - if true, the piglin just hunted and cannot hunt.
             [Long] ttl: The number of game ticks before this memory is removed.
         [NBT Compound / JSON Object] minecraft:universal_anger: If the piglin is being universally angered. Only set when universal anger gamerule is enabled.
             [Boolean] value: 1 or 0 (true/false) - if true, universally angered.
             [Long] ttl: The number of game ticks before this memory is removed.
         Piglin Brute memories:[]
         [NBT Compound / JSON Object] minecraft:angry_at: The target of this piglin brute. Set after a piglin or piglin brute is attacked.
             [Int Array] value: Universally Unique IDentifier of the entity that the piglin targets, stored as four ints.
             [Long] ttl: The number of game ticks before this memory is removed.
         [NBT Compound / JSON Object] minecraft:home: Where the piglin brute's patrol point is. Set after spawning as part of a bastion remnant during world generation. Piglin brutes will limit their pathfinding to stay within this area.
             [NBT Compound / JSON Object] value: The value of this memory.
                 [String] dimension: The dimension ID of the patrol point.
                 [Int Array] pos: The X, Y, and Z coordinates of the patrol point.
         Sniffer memories:[]
         [NBT Compound / JSON Object] minecraft:sniffer_explored_positions: The last 20 positions in which the sniffer has dug. The sniffer cannot dig in these positions.
             [NBT List / JSON Array] value: The value of the memory.
                 [NBT Compound / JSON Object]: A single block.
                     [String] dimension: The dimension ID of the block.
                     [Int Array] pos: The X, Y, and Z coordinates of the block.
         [NBT Compound / JSON Object] minecraft:sniff_cooldown: The sniffer does not sniff if this memory exists.
             [NBT Compound / JSON Object] value: An empty compound tag.
             [Long] ttl: The number of game ticks before this memory is removed, initially set to 100 game ticks (5 seconds).
         Villager memories:[]
         [NBT Compound / JSON Object] minecraft:home: Where this villager's bed is.
             [NBT Compound / JSON Object] value: The value of this memory.
                 [String] dimension: The dimension ID of the bed.
                 [Int Array] pos: The X, Y, and Z coordinates of the bed.
         [NBT Compound / JSON Object] minecraft:job_site: Where this villager's job site block is.
             [NBT Compound / JSON Object] value: The value of this memory.
                 [String] dimension: The dimension ID of the job site block.
                 [Int Array] pos: The X, Y, and Z coordinates of the job site block.
         [NBT Compound / JSON Object] minecraft:last_slept: The game tick that the villager last slept in a bed.
             [Long] value: The value of this memory.
         [NBT Compound / JSON Object] minecraft:last_woken: The game tick that the villager last woke up from a bed.
             [Long] value: The value of this memory.
         [NBT Compound / JSON Object] minecraft:last_worked_at_poi: The game tick that the villager last worked at their job site.
             [Long] value: The value of this memory.
         [NBT Compound / JSON Object] minecraft:meeting_point: Where this villager's meeting point is.
             [NBT Compound / JSON Object] value: The value of this memory.
                 [String] dimension: The dimension ID of the meeting point.
                 [Int Array] pos: The X, Y, and Z coordinates of the meeting point.
         [NBT Compound / JSON Object] minecraft:potential_job_site: Where this villager's potential job site block is.
             [NBT Compound / JSON Object] value: The value of this memory.
                 [String] dimension: The dimension ID of the potential job site block.
                 [Int Array] pos: The X, Y, and Z coordinates of the potential job site block.
         [NBT Compound / JSON Object] minecraft:golem_detected_recently: If the villager has detected an iron golem recently.
             [Boolean] value: 1 or 0 (true/false) - if true, the villager just saw an iron golem.
             [Long] ttl: The number of game ticks before this memory is removed.
         Warden memories:[]
         [NBT Compound / JSON Object] minecraft:is_emerging: Exists if the warden is emerging.
             [NBT Compound / JSON Object] value: An empty compound tag.
         [NBT Compound / JSON Object] minecraft:dig_cooldown: The warden doesn't dig down if this memory exists.
             [NBT Compound / JSON Object] value: An empty compound tag.
             [Long] ttl: The number of game ticks before this memory is removed, initially set to 1200 game ticks (60 seconds).
         [NBT Compound / JSON Object] minecraft:is_sniffing: Exists if the warden is sniffing.
             [NBT Compound / JSON Object] value: An empty compound tag.
         [NBT Compound / JSON Object] minecraft:recent_projectile: Exists if the warden was stimulated by a projectile recently.
             [NBT Compound / JSON Object] value: An empty compound tag.
             [Long] ttl: The number of game ticks before this memory is removed, initially set to 100 game ticks (5 seconds).
         [NBT Compound / JSON Object] minecraft:roar_sound_cooldown: The warden doesn't roar if this memory exists.
             [NBT Compound / JSON Object] value: An empty compound tag.
             [Long] ttl: The number of game ticks before this memory is removed, initially set to 60 game ticks (3 seconds).
         [NBT Compound / JSON Object] minecraft:roar_sound_delay: The warden doesn't roar if this memory exists.
             [NBT Compound / JSON Object] value: An empty compound tag.
             [Long] ttl: The number of game ticks before this memory is removed, initially set to 25 game ticks (1.25 seconds).
         [NBT Compound / JSON Object] minecraft:touch_cooldown: The warden doesn't increase anger at an entity by touching it if this memory exists.
             [NBT Compound / JSON Object] value: An empty compound tag.
             [Long] ttl: The number of game ticks before this memory is removed, initially set to 20 game ticks (1 second).
         [NBT Compound / JSON Object] minecraft:vibration_cooldown: The warden doesn't listen for vibrations if this memory exists.
             [NBT Compound / JSON Object] value: An empty compound tag.
             [Long] ttl: The number of game ticks before this memory is removed, initially set to 40 game ticks (2 seconds).
         [NBT Compound / JSON Object] minecraft:sniff_cooldown: The warden does not sniff if this memory exists.
             [NBT Compound / JSON Object] value: An empty compound tag.
             [Long] ttl: The number of game ticks before this memory is removed, initially set to 100 game ticks (5 seconds).
     *
     *
     */
};
struct mob_drop_chances_t {
    float head = 0.085f;
    float chest = 0.085f;
    float legs = 0.085f;
    float feet = 0.085f;
    float mainhand = 0.085f;
    float offhand = 0.085f;
    float body = 0.085f;
    float saddle = 0.085f;
};
struct mob_equipment_t {
    std::optional<item_stack_t> head;
    std::optional<item_stack_t> chest;
    std::optional<item_stack_t> legs;
    std::optional<item_stack_t> feet;
    std::optional<item_stack_t> mainhand;
    std::optional<item_stack_t> offhand;
    std::optional<item_stack_t> body;
    std::optional<item_stack_t> saddle;
};

struct struct_mob_leash_t {
    std::array<std::int32_t, 4> UUID;
};
using mob_leash_t = std::variant<struct_mob_leash_t, std::array<std::int32_t, 4>>;

struct mob_locator_bar_icon_t {
    std::int32_t color;
    std::string style;
};

struct generic_entity_format_t {
    std::optional<short> Air;
    std::optional<text_t> CustomName;
    bool CustomNameVisible = true;
    std::optional<glz::generic> data;
    std::optional<double> fall_distance;
    short Fire = -20;
    bool Glowing = false;
    bool HasVisualFire = false;
    bool Invulnerable = false;
    resource_location_t id;
    std::optional<std::int32_t> invulnerable_time;
    std::optional<std::array<double, 3>> Motion;
    bool NoGravity = false;
    bool OnGround = true;
    std::optional<std::vector<wrapped_entity_format_t>> Passengers;
    std::optional<std::int32_t> PortalCooldown;
    std::optional<std::array<double, 3>> Pos;
    std::optional<std::array<float, 2>> Rotation;
    bool Silent = false;
    std::vector<std::string> Tags = {};
    std::optional<std::int32_t> TicksFrozen;
    std::optional<std::array<std::int32_t, 4>> UUID;
};

struct generic_mob_entity_format_t : generic_entity_format_t {
    float AbsorptionAmount = 0.0f;
    std::vector<mob_attribute_t> attributes;
    mob_brain_t Brain = mob_brain_t();
    bool CanPickUpLoot = true;
    std::optional<std::vector<double>> current_explosion_impact_pos;
    std::optional<std::int32_t> current_impulse_context_reset_grace_time;
    std::optional<resource_location_t> DeathLootTable;
    std::optional<std::int64_t> DeathLootTableSeed;
    std::optional<std::int16_t> DeathTime;
    mob_drop_chances_t drop_chances = mob_drop_chances_t();
    mob_equipment_t equipment = mob_equipment_t();
    std::optional<std::int8_t> FallFlying;
    std::optional<float> Health;
    std::optional<std::array<std::int32_t, 3>> home_pos;
    std::optional<std::int32_t> home_radius;
    std::optional<short> HurtTime;
    std::optional<std::array<std::int32_t, 4>> last_hurt_by_mob;
    std::optional<std::int32_t> ticks_since_last_hurt_by_mob;
    std::optional<std::array<std::int32_t, 4>> last_hurt_by_player;
    std::optional<std::int32_t> last_hurt_by_player_memory_time;
    std::optional<mob_leash_t> leash;
    bool LeftHanded = false;
    std::optional<mob_locator_bar_icon_t> locator_bar_icon;
    bool NoAI = false;
    bool PersistenceRequired = false;
    std::optional<std::array<std::int32_t, 3>> sleeping_pos;
    std::optional<std::string> Team;
};
struct breeding_mob_entity_format_t : generic_mob_entity_format_t {
    std::optional<std::int32_t> Age;
    bool AgeLocked = false;
    std::optional<std::int32_t> ForcedAge;
    std::optional<std::int32_t> InLove;
    std::optional<std::array<std::int32_t, 4>> LoveCause;
};
struct generic_horse_mob_entity_format_t : breeding_mob_entity_format_t {
    bool Bred = false;
    bool EatingHaystack = false;
    std::optional<std::array<std::int32_t, 4>> Owner;
    bool Tame = false;
    std::optional<std::int32_t> Temper;
};
struct cube_mob_entity_format_t : generic_mob_entity_format_t {
    std::optional<int> Size;
    bool wasOnGround = true;
};
struct entity_can_be_angered_t {
    std::optional<std::array<std::int32_t, 4>> angry_at;
    std::optional<std::int64_t> anger_end_time;
};
struct entity_can_be_tamed_t {
    std::optional<std::array<std::int32_t, 4>> Owner;
    bool Sitting = false;
};
struct raider_mob_entity_format_t : generic_mob_entity_format_t {
    bool CanJoinRaid = true;
    bool PatrolLeader = false;
    bool Patrolling = false;
    std::optional<std::array<std::int32_t, 3>> patrol_target;
    std::optional<std::int32_t> RaidId;
    std::optional<std::int32_t> Wave;
};
struct item_based_steerable_t {
    struct item_based_steering_t {
        bool boosting = false;
        std::optional<int> boost_time;
        std::optional<int> boost_time_total;
    };
    std::optional<item_based_steering_t> item_based_steering;
};

struct allay_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "allay"; static constexpr std::string_view type_key = "id";

    std::int64_t DuplicationCooldown;
    std::vector<item_stack_t> Inventory;
    struct listener_t {
        struct listener_event_t {
            std::int32_t distance;
            resource_location_t game_event;
            std::array<double, 3> pos;
            std::optional<std::array<std::int32_t, 4>> projectile_owner;
            std::optional<std::array<std::int32_t, 4>> source;
        };

        std::int32_t distance;
        std::optional<listener_event_t> event;
        std::int32_t event_delay;
        std::int32_t event_distance;
        std::int32_t range;

        struct source_block_t {
            static constexpr std::string_view type_tag = "block";

            std::array<std::int32_t, 3> pos;
        };
        struct source_entity_t {
            static constexpr std::string_view type_tag = "entity";

            std::array<std::int32_t, 4> source_entity;
            float y_offset;
        };
        std::variant<source_block_t, source_entity_t> source;
    } listener;
};
struct armadillo_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "armadillo"; static constexpr std::string_view type_key = "id";

    std::int32_t scute_time = 12000;
    std::string state = "idle"; // idle || unrolling || scared
};
struct armor_stand_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "armor_stand"; static constexpr std::string_view type_key = "id";

    std::optional<std::int32_t> DisabledSlots;
    bool Invisible = false;
    bool Marker = false;
    bool NoBasePlate = false;
    struct pose_t {
        std::optional<std::array<float, 3>> Body;
        std::optional<std::array<float, 3>> Head;
        std::optional<std::array<float, 3>> LeftArm;
        std::optional<std::array<float, 3>> LeftLeg;
        std::optional<std::array<float, 3>> RightArm;
        std::optional<std::array<float, 3>> RightLeg;
    };
    std::optional<pose_t> Pose;
    bool ShowArms = false;
    bool Small = false;
};
struct axolotl_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "axolotl"; static constexpr std::string_view type_key = "id";
    bool FromBucket = false;
    std::int32_t Variant;
};
struct bat_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "bat"; static constexpr std::string_view type_key = "id";

    bool BatFlags;
};
struct bee_mob_entity_format_t : breeding_mob_entity_format_t, entity_can_be_angered_t {
    static constexpr std::string_view type_tag = "bee"; static constexpr std::string_view type_key = "id";

    std::int32_t CannotEnterHiveTicks = 0;
    std::int32_t CropsGrownSincePollination = 0;
    bool HasNectar = false;
    bool HasStung = false;
    std::optional<std::array<std::int32_t, 3>> flower_pos;
    std::optional<std::array<std::int32_t, 3>> hive_pos;
    std::int32_t TicksSincePollination = 0;
};
struct blaze_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "blaze"; static constexpr std::string_view type_key = "id";
};
struct bogged_mob_entity_format_t : generic_mob_entity_format_t{
    static constexpr std::string_view type_tag = "bogged"; static constexpr std::string_view type_key = "id";

    bool sheared;
};
struct breeze_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "breeze"; static constexpr std::string_view type_key = "id";
};
struct camel_mob_entity_format_t : generic_horse_mob_entity_format_t {
    static constexpr std::string_view type_tag = "camel"; static constexpr std::string_view type_key = "id";
    std::optional<std::int64_t> LastPoseTick;
};
struct camel_husk_mob_entity_format_t : generic_horse_mob_entity_format_t {
    static constexpr std::string_view type_tag = "camel_husk"; static constexpr std::string_view type_key = "id";
    std::optional<std::int64_t> LastPoseTick;
};
struct cat_mob_entity_format_t : breeding_mob_entity_format_t, entity_can_be_tamed_t {
    static constexpr std::string_view type_tag = "cat"; static constexpr std::string_view type_key = "id";
    std::int8_t CollarColor = 14;
    std::string sound_variant;
    std::string variant;
};
struct chicken_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "chicken"; static constexpr std::string_view type_key = "id";

    std::int32_t EggLayTime = 12000;
    bool IsChickenJockey = false;
    std::optional<std::string> sound_variant;
    std::optional<std::string> variant;
};
struct cod_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "cod"; static constexpr std::string_view type_key = "id";
    bool FromBucket = false;
};
struct copper_golem_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "copper_golem"; static constexpr std::string_view type_key = "id";

    std::string weather_state = "unaffected"; // unaffected || exposed || weathered || oxidized
    std::int64_t next_weather_age = -1;
};
struct cow_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "cow"; static constexpr std::string_view type_key = "id";
    std::optional<std::string> sound_variant;
    std::optional<std::string> variant;
};
struct creaking_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "creaking"; static constexpr std::string_view type_key = "id";
};
struct creeper_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "creeper"; static constexpr std::string_view type_key = "id";

    bool ExplosionRadius = 3;
    short Fuse = 30;
    bool ignited = false;
    bool powered = false;
};
struct dolphin_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "dolphin"; static constexpr std::string_view type_key = "id";

    std::int32_t Moistness = 2400;
    bool GotFish = false;
};
struct donkey_mob_entity_format_t : generic_horse_mob_entity_format_t {
    static constexpr std::string_view type_tag = "donkey"; static constexpr std::string_view type_key = "id";

    bool ChestedHorse = false;
    std::optional<std::vector<item_stack_t>> Items;
};
struct drowned_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "drowned"; static constexpr std::string_view type_key = "id";

    bool CanBreakDoors = false;
    std::int32_t DrownedConversionTime = -1;
    std::int32_t FrostbiteConversionTime = -1;
    std::int32_t InWaterTime = -1;
    bool IsBaby = false;
};
struct elder_guardian_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "elder_guardian"; static constexpr std::string_view type_key = "id";
};
struct ender_dragon_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "ender_dragon"; static constexpr std::string_view type_key = "id";

    /*
     * phases
     *  0 - circling
     *  1 - strafing to shoot fireball
     *  2 - flying to portal to land
     *  3 - landing on portal
     *  4 - taking off from portal
     *  5 - landed, breath attack
     *  6 - landed, looking for a player for breath attack
     *  7 - landed, roar before breath attack
     *  8 - charging player
     *  9 - goin' to die!
     *  10 - hovering and being braindead
     */

    std::int32_t DragonDeathTime = 0;
    std::int32_t DragonPhase = 10;
    float sitting_damage_received = 0;
};
struct enderman_mob_entity_format_t : generic_mob_entity_format_t, entity_can_be_angered_t {
    static constexpr std::string_view type_tag = "enderman"; static constexpr std::string_view type_key = "id";
    block_state_t carriedBlockState;
};
struct endermite_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "endermite"; static constexpr std::string_view type_key = "id";
    std::int32_t Lifetime = 0;
};
struct evoker_mob_entity_format_t : raider_mob_entity_format_t {
    static constexpr std::string_view type_tag = "evoker"; static constexpr std::string_view type_key = "id";
    std::int32_t SpellTicks = 0;
};
struct fox_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "fox"; static constexpr std::string_view type_key = "id";

    bool Crouching = false;
    bool Sitting = false;
    bool Sleeping = false;
    std::vector<std::array<std::int32_t, 4>> Trusted = {};
    std::optional<std::string> Type;
};
struct frog_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "frog"; static constexpr std::string_view type_key = "id";
    std::optional<std::string> variant;
};
struct frostbite_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "frostbite"; static constexpr std::string_view type_key = "id";
    bool CanBreakDoors = false;
    std::int32_t DrownedConversionTime = -1;
    std::int32_t InWaterTime = -1;
    bool IsBaby = false;
};
struct ghast_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "ghast"; static constexpr std::string_view type_key = "id";
    std::int8_t ExplosionPower = 1;
};
struct giant_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "giant"; static constexpr std::string_view type_key = "id";
};
struct glow_squid_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "glow_squid"; static constexpr std::string_view type_key = "id";
    std::int32_t DarkTicksRemaining = 0;
};
struct goat_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "goat"; static constexpr std::string_view type_key = "id";
    bool HasLeftHorn = true;
    bool HasRightHorn = true;
    bool IsScreamingGoat = false;
};
struct guardian_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "guardian"; static constexpr std::string_view type_key = "id";
};
struct happy_ghast_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "happy_ghast"; static constexpr std::string_view type_key = "id";
    std::int32_t still_timeout = 10;
};
struct hoglin_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "hoglin"; static constexpr std::string_view type_key = "id";
    bool CannotBeHunted = false;
    bool IsImmuneToZombification = false;
    std::int32_t TimeInOverworld = 0;
};
struct husk_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "husk"; static constexpr std::string_view type_key = "id";
    bool CanBreakDoors = false;
    std::int32_t DrownedConversionTime = -1;
    std::int32_t FrostbiteConversionTime = -1;
    std::int32_t InWaterTime = -1;
    bool IsBaby = false;
};
struct illusioner_mob_entity_format_t : raider_mob_entity_format_t {
    static constexpr std::string_view type_tag = "illusioner"; static constexpr std::string_view type_key = "id";
    std::int32_t SpellTicks = 0;
};
struct iron_golem_mob_entity_format_t : generic_mob_entity_format_t, entity_can_be_angered_t {
    static constexpr std::string_view type_tag = "iron_golem"; static constexpr std::string_view type_key = "id";
    bool PlayerCreated = false;
};
struct llama_mob_entity_format_t : generic_horse_mob_entity_format_t {
    static constexpr std::string_view type_tag = "llama"; static constexpr std::string_view type_key = "id";
    bool ChestedHorse = false;
    std::optional<std::int32_t> DespawnDelay;
    std::optional<std::vector<item_stack_t>> Items;
    std::int32_t Strength = 3;
    std::int32_t Variant;
};
struct magma_cube_mob_entity_format_t : cube_mob_entity_format_t {
    static constexpr std::string_view type_tag = "magma_cube"; static constexpr std::string_view type_key = "id";
};
struct mannequin_mob_entity_format_t {
    static constexpr std::string_view type_tag = "mannequin"; static constexpr std::string_view type_key = "id";
    std::variant<std::string, game_profile_t> profile;
    std::vector<std::string> hidden_layers; // cape || jacket || left_sleeve || right_sleeve || hat || left_pants_leg || right_pants_leg
    std::string main_hand; // left || right
    std::string pose; // standing || crouching || swimming || fall_flying || sleeping
    bool immovable = false;
    bool hide_description = false;
    std::optional<text_t> description;
};

struct suspicious_stew_effect_t {
    std::optional<resource_location_t> id;
    std::optional<std::int32_t> duration;
};
struct mooshroom_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "mooshroom"; static constexpr std::string_view type_key = "id";
    std::string Type;
    std::vector<suspicious_stew_effect_t> stew_effects = {};
};

struct mule_mob_entity_format_t : generic_horse_mob_entity_format_t {
    static constexpr std::string_view type_tag = "mule"; static constexpr std::string_view type_key = "id";
    bool ChestedHorse = false;
    std::optional<std::vector<item_stack_t>> Items;
};
struct nautilus_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "nautilus"; static constexpr std::string_view type_key = "id";
};
struct ocelot_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "ocelot"; static constexpr std::string_view type_key = "id";
    bool Trusting = false;
};
struct panda_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "panda"; static constexpr std::string_view type_key = "id";
    std::optional<std::string> HiddenGene;
    std::optional<std::string> MainGene;
};
struct parrot_mob_entity_format_t : generic_mob_entity_format_t, entity_can_be_tamed_t {
    static constexpr std::string_view type_tag = "parrot"; static constexpr std::string_view type_key = "id";
    std::int32_t Variant;
};
struct phantom_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "phantom"; static constexpr std::string_view type_key = "id";
    std::int32_t size = 0;
    std::optional<std::array<std::int32_t, 3>> anchor_pos;
};
struct pig_mob_entity_format_t : breeding_mob_entity_format_t, item_based_steerable_t {
    static constexpr std::string_view type_tag = "pig"; static constexpr std::string_view type_key = "id";
    std::optional<std::string> sound_variant;
    std::optional<std::string> variant;
};
struct piglin_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "piglin"; static constexpr std::string_view type_key = "id";
    bool CannotHunt = false;
    std::optional<std::vector<item_stack_t>> Inventory;
    bool IsBaby = false;
    bool IsImmuneToZombification = false;
    std::int32_t TimeInOverworld = 0;
};
struct piglin_brute_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "piglin_brute"; static constexpr std::string_view type_key = "id";
    bool IsImmuneToZombification = false;
    std::int32_t TimeInOverworld = 0;
};
struct pillager_mob_entity_format_t : raider_mob_entity_format_t {
    static constexpr std::string_view type_tag = "pillager"; static constexpr std::string_view type_key = "id";
    std::optional<std::vector<item_stack_t>> Inventory;
};
struct player_mob_entity_format_t {
    static constexpr std::string_view type_tag = "player"; static constexpr std::string_view type_key = "id";
    struct abilities_t {
        bool flying = false;
        float flySpeed = 0.05f;
        bool instabuild = false;
        bool invulnerable = false;
        bool mayBuild = true;
        bool mayfly = false;
        float walkSpeed = 0.1f;
    } abilities;
    std::int32_t DataVersion;
    resource_location_t Dimension;
    std::vector<item_stack_t> EnderItems;
    std::optional<std::array<double, 3>> entered_nether_pos;
    float foodExhaustionLevel = 0;
    std::int32_t foodLevel = 20;
    float foodSaturationLevel = 5;
    std::int32_t foodTickTimer;
    bool ignore_fall_damage_from_current_explosion = false;
    std::vector<item_stack_t> Inventory;
    struct last_death_location_t {
        resource_location_t dimension;
        std::array<std::int32_t, 3> pos;
    };
    std::optional<last_death_location_t> LastDeathLocation;
    std::int32_t playerGameType = 0;
    std::int32_t previousPlayerGameType = 0;
    struct recipe_book_t {
        std::vector<resource_location_t> recipes = {};
        std::vector<resource_location_t> toBeDisplayed = {};
        bool isFilteringCraftable = false;
        bool isGuiOpen = false;
        bool isFurnaceFilteringCraftable = false;
        bool isFurnaceGuiOpen = false;
        bool isBlastingFurnaceFilteringCraftable = false;
        bool isBlastingFurnaceGuiOpen = false;
        bool isSmokerFilteringCraftable = false;
        bool isSmokerGuiOpen = false;
    };
    recipe_book_t recipeBook = recipe_book_t();
    struct respawn_t {
        std::array<std::int32_t, 3> pos;
        float yaw = 0.0f;
        resource_location_t dimension;
        float pitch;
        bool forced = false;
    };
    std::optional<respawn_t> respawn;
    struct root_vehicle_t {
        std::array<std::int32_t, 4> Attach;
        std::shared_ptr<wrapped_entity_format_t> Entity;
    };
    std::optional<root_vehicle_t> RootVehicle;
    std::int32_t Score = 0;
    bool seenCredits = false;
    item_stack_t SelectedItem;
    std::int32_t SelectedItemSlot = 0;
    std::shared_ptr<wrapped_entity_format_t> ShoulderEntityLeft;
    std::shared_ptr<wrapped_entity_format_t> ShoulderEntityRight;
    short SleepTimer = 0;
    bool spawn_extra_particles_on_fall = true;
    struct warden_spawn_tracker_t {
        std::int32_t warning_level = 0;
        std::int32_t cooldown_ticks = 0;
        std::int32_t ticks_since_last_warning = 0;
    } warden_spawn_tracker;
    std::int32_t XpLevel = 0;
    float Xp = 0;
    std::int32_t XpSeed;
    std::int32_t XpTotal = 0;
};
struct polar_bear_mob_entity_format_t : breeding_mob_entity_format_t, entity_can_be_angered_t {
    static constexpr std::string_view type_tag = "polar_bear"; static constexpr std::string_view type_key = "id";
};
struct pufferfish_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "pufferfish"; static constexpr std::string_view type_key = "id";
    bool FromBucket = false;
    std::int32_t PuffState = 0; // 0 - deflated, 1 - halfway, 2 - full
};
struct rabbit_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "rabbit"; static constexpr std::string_view type_key = "id";
    std::int32_t MoreCarrotTicks = 0;
    std::int32_t RabbitType;
};
struct ravager_mob_entity_format_t : raider_mob_entity_format_t {
    static constexpr std::string_view type_tag = "ravager"; static constexpr std::string_view type_key = "id";
    std::int32_t AttackTick = 0;
    std::int32_t RoarTick = 0;
    std::int32_t StunTick = 0;
};
struct salmon_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "salmon"; static constexpr std::string_view type_key = "id";
    bool FromBucket = false;
    std::string type; // small || medium || large
};
struct sheep_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "sheep"; static constexpr std::string_view type_key = "id";
    std::int8_t Color = 0;
    bool Sheared;
};
struct shulker_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "shulker"; static constexpr std::string_view type_key = "id";
    std::int8_t AttachFace = 0; // 1 - bottom, 2 - north, 3 - south, 4 - west, 5 - east, 0 - up
    std::int8_t Color = 0;
    std::optional<std::int8_t> Peek;

    /*
     * The "height" of the shulker at any Peek value is calculated via the equation 1.5 - (cos((((Peek - 128) % 256) - 128) * pi / 100) / 2) with % being the modulo function.
     * This "height" goes in the direction that the shulker is facing according to AttachFace.
     */
};
struct silverfish_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "silverfish"; static constexpr std::string_view type_key = "id";
};
struct skeleton_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "skeleton"; static constexpr std::string_view type_key = "id";
    std::int32_t StrayConversionTime = -1;
};
struct skeleton_horse_mob_entity_format_t : generic_horse_mob_entity_format_t {
    static constexpr std::string_view type_tag = "skeleton_horse"; static constexpr std::string_view type_key = "id";
    bool SkeletonTrap = false;
    std::int32_t SkeletonTrapTime = 0;
};
struct horse_mob_entity_format_t : generic_horse_mob_entity_format_t {
    static constexpr std::string_view type_tag = "horse"; static constexpr std::string_view type_key = "id";
    std::int32_t Variant;
};
struct slime_mob_entity_format_t : cube_mob_entity_format_t {
    static constexpr std::string_view type_tag = "slime"; static constexpr std::string_view type_key = "id";
};
struct sniffer_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "sniffer"; static constexpr std::string_view type_key = "id";
};
struct snow_golem_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "snow_golem"; static constexpr std::string_view type_key = "id";
    bool Pumpkin = true;
};
struct spider_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "spider"; static constexpr std::string_view type_key = "id";
};
struct strider_mob_entity_format_t : breeding_mob_entity_format_t, item_based_steerable_t {
    static constexpr std::string_view type_tag = "strider"; static constexpr std::string_view type_key = "id";
};
struct squid_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "squid"; static constexpr std::string_view type_key = "id";
};
struct stray_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "stray"; static constexpr std::string_view type_key = "id";
};
struct sulfur_cube_mob_entity_format_t : cube_mob_entity_format_t {
    static constexpr std::string_view type_tag = "sulfur_cube"; static constexpr std::string_view type_key = "id";

    std::int32_t pickup_timer = 0;
    bool from_bucket = false;
    std::int32_t fuse = -1;
};
struct tadpole_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "tadpole"; static constexpr std::string_view type_key = "id";
    std::int32_t Age = 0;
    bool FromBucket = false;
};
struct trader_llama_mob_entity_format_t : generic_horse_mob_entity_format_t {
    static constexpr std::string_view type_tag = "trader_llama"; static constexpr std::string_view type_key = "id";
    bool ChestedHorse = false;
    std::optional<std::int32_t> DespawnDelay;
    std::optional<std::vector<item_stack_t>> Items;
    std::int32_t Strength = 3;
    std::int32_t Variant;
};
struct tropical_fish_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "tropical_fish"; static constexpr std::string_view type_key = "id";
    bool FromBucket = false;
    std::int32_t Variant;
};
struct turtle_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "turtle"; static constexpr std::string_view type_key = "id";
    bool has_egg = false;
};
struct vex_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "vex"; static constexpr std::string_view type_key = "id";
    std::optional<std::array<std::int32_t, 3>> bound_pos;
    std::optional<std::int32_t> life_ticks;
    std::optional<std::array<std::int32_t, 4>> owner;
};
struct villager_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "villager"; static constexpr std::string_view type_key = "id";
    std::int8_t FoodLevel;
    struct gossip_t {
        std::int32_t Value;
        std::array<std::int32_t, 4> Target;
        std::string type; // major_negative || major_positive || trading || minor_negative || minor_positive
    };
    std::vector<gossip_t> gossips;
    std::vector<item_stack_t> Inventory;
    std::int64_t LastGossipDecay;
    std::int64_t LastRestock;
    std::int32_t RestocksToday;
    bool VillagerDataFinalized;
    std::int32_t Xp;
    struct villager_data_t {
        std::int32_t level = 1;
        resource_location_t profession;
        resource_location_t type;
    } VillagerData;
    struct offers_t {
        struct recipe_t {
            item_stack_t buy;
            item_stack_t buyB;
            std::int32_t demand;
            std::int32_t maxUses;
            float priceMultiplier;
            bool rewardExp = true;
            item_stack_t sell;
            std::int32_t specialPrice;
            std::int32_t uses;
            std::int32_t xp;
        };
        std::vector<recipe_t> Recipes;
    } Offers;
};
struct vindicator_mob_entity_format_t : raider_mob_entity_format_t {
    static constexpr std::string_view type_tag = "vindicator"; static constexpr std::string_view type_key = "id";
    bool Johnny = false;
};
struct wandering_trader_mob_entity_format_t : breeding_mob_entity_format_t {
    static constexpr std::string_view type_tag = "wandering_trader"; static constexpr std::string_view type_key = "id";
    std::optional<std::vector<item_stack_t>> Inventory;
    std::int64_t DespawnDelay = 0;
    std::optional<std::array<std::int32_t, 4>> wander_target;
    struct offers_t {
        struct recipe_t {
            item_stack_t buy;
            item_stack_t buyB;
            std::int32_t maxUses;
            bool rewardExp;
            item_stack_t sell;
            std::int32_t uses;
        };
        std::vector<recipe_t> Recipes;
    } Offers;
};
struct warden_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "warden"; static constexpr std::string_view type_key = "id";
    struct anger_t {
        struct suspect_t {
            std::int32_t anger;
            std::array<std::int32_t, 4> uuid;
        };
        std::vector<suspect_t> suspects = {};
    };
    struct listener_t {
        struct listener_event_t {
            float distant;
            resource_location_t game_event;
            std::array<double, 3> pos;
            std::optional<std::array<std::int32_t, 4>> projectile_owner;
            std::optional<std::array<std::int32_t, 4>> source;
        };

        listener_event_t event;
        std::int32_t event_delay;
        struct selector_t {
            std::int64_t tick;
            listener_event_t event;
        } selector;
    } listener;

    anger_t anger = anger_t();
};
struct witch_mob_entity_format_t : raider_mob_entity_format_t {
    static constexpr std::string_view type_tag = "witch"; static constexpr std::string_view type_key = "id";
};
struct wither_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "wither"; static constexpr std::string_view type_key = "id";
    std::int32_t Invul = 220;
};
struct wither_skeleton_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "wither_skeleton"; static constexpr std::string_view type_key = "id";
};
struct wolf_mob_entity_format_t : breeding_mob_entity_format_t, entity_can_be_angered_t, entity_can_be_tamed_t {
    static constexpr std::string_view type_tag = "wolf"; static constexpr std::string_view type_key = "id";
    std::int8_t CollarColor = 14;
    std::string sound_variant;
    std::string variant;
};
struct zoglin_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "zoglin"; static constexpr std::string_view type_key = "id";
    bool IsBaby = false;
};
struct zombie_mob_entity_format_t  : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "zombie"; static constexpr std::string_view type_key = "id";
    bool CanBreakDoors = false;
    std::int32_t DrownedConversionTime = -1;
    std::int32_t FrostbiteConversionTime = -1;
    std::int32_t InWaterTime = -1;
    bool IsBaby = false;
};
struct zombie_horse_mob_entity_format_t : generic_horse_mob_entity_format_t {
    static constexpr std::string_view type_tag = "zombie_horse"; static constexpr std::string_view type_key = "id";
};
struct zombie_nautilus_mob_entity_format_t : generic_mob_entity_format_t, entity_can_be_tamed_t {
    static constexpr std::string_view type_tag = "zombie_nautilus"; static constexpr std::string_view type_key = "id";
    std::string variant;
};
struct zombie_villager_mob_entity_format_t : generic_mob_entity_format_t {
    static constexpr std::string_view type_tag = "zombie_villager"; static constexpr std::string_view type_key = "id";
    struct gossip_t {
        std::int32_t Value;
        std::array<std::int32_t, 4> Target;
        std::string type; // major_negative || major_positive || trading || minor_negative || minor_positive
    };

    std::vector<gossip_t> gossips;
    std::vector<item_stack_t> Inventory;
    std::int64_t LastGossipDecay;
    std::int64_t LastRestock;
    std::int32_t RestocksToday;
    bool VillagerDataFinalized;
    std::int32_t Xp;
    struct villager_data_t {
        std::int32_t level;
        resource_location_t profession;
        resource_location_t type;
    } VillagerData;
    struct offers_t {
        struct recipe_t {
            item_stack_t buy;
            item_stack_t buyB;
            std::int32_t demand;
            std::int32_t maxUses;
            float priceMultiplier;
            bool rewardExp;
            item_stack_t sell;
            std::int32_t specialPrice;
            std::int32_t uses;
            std::int32_t xp;
        };
        std::vector<recipe_t> Recipes;
    } Offers;
    bool CanBreakDoors = false;
    std::int32_t DrownedConversionTime = -1;
    std::int32_t InWaterTime = -1;
    bool IsBaby = false;
    std::int32_t ConversionTime = -1;
    std::optional<std::array<std::int32_t, 4>> ConversionPlayer;
};
struct zombified_piglin_mob_entity_format_t : generic_mob_entity_format_t, entity_can_be_angered_t {
    static constexpr std::string_view type_tag = "zombified_piglin"; static constexpr std::string_view type_key = "id";
    bool CanBreakDoors = false;
    std::int32_t DrownedConversionTime = -1;
    std::int32_t InWaterTime = -1;
    bool IsBaby = false;
};

struct generic_projectile_entity_format_t : generic_entity_format_t {
    bool HasBeenShot = true;
    bool LeftOwner = true;
    std::optional<std::array<std::int32_t, 4>> Owner;
};
struct hurting_projectile_entity_format_t : generic_projectile_entity_format_t {
    double acceleration_power = 1;
};
struct arrow_projectile_entity_format_t : hurting_projectile_entity_format_t {
    bool crit = false;
    double damage = 2;
    std::optional<block_state_t> inBlockState;
    bool inGround = false;
    short life = 0;
    std::int8_t pickup = 1; // 0 = none, 1 = allow, 2 = creative
    std::optional<std::int8_t> PierceLevel;
    std::optional<std::int8_t> shake;
    bool ShotFromCrossbow = false;
    std::optional<resource_location_t> SoundEvent;
    item_stack_t item;
    item_stack_t weapon;
};

struct arrow_entity_format_t : arrow_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "arrow"; static constexpr std::string_view type_key = "id";
};
struct breeze_wind_charge_entity_format_t : hurting_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "breeze_wind_charge"; static constexpr std::string_view type_key = "id";
};
struct dragon_fireball_entity_format_t : hurting_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "dragon_fireball"; static constexpr std::string_view type_key = "id";
};
struct egg_entity_format_t : generic_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "egg"; static constexpr std::string_view type_key = "id";
    item_stack_t Item;
};
struct ender_pearl_entity_format_t : generic_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "ender_pearl"; static constexpr std::string_view type_key = "id";
    item_stack_t Item;
};
struct experience_bottle_entity_format_t : generic_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "experience_bottle"; static constexpr std::string_view type_key = "id";
    item_stack_t Item;
};
struct fireball_entity_format_t : hurting_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "fireball"; static constexpr std::string_view type_key = "id";
    std::int8_t ExplosionPower = 1;
    item_stack_t Item;
};
struct firework_rocket_entity_format_t : generic_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "firework_rocket"; static constexpr std::string_view type_key = "id";
    item_stack_t FireworksItem;
    std::int32_t Life = 0;
    std::optional<std::int32_t> LifeTime;
    bool ShotAtAngle = false;
};
struct lingering_potion_entity_format_t : generic_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "lingering_potion"; static constexpr std::string_view type_key = "id";
    item_stack_t Item;
};
struct llama_spit_entity_format_t : generic_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "llama_spit"; static constexpr std::string_view type_key = "id";
};
struct shulker_bullet_entity_format_t : generic_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "shulker_bullet"; static constexpr std::string_view type_key = "id";
    std::optional<std::int32_t> Steps;
    std::optional<std::array<std::int32_t, 4>> Target;
    std::optional<double> TXD;
    std::optional<double> TYD;
    std::optional<double> TZD;
};
struct small_fireball_entity_format_t : hurting_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "small_fireball"; static constexpr std::string_view type_key = "id";
    item_stack_t Item;
};
struct snowball_entity_format_t : generic_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "snowball"; static constexpr std::string_view type_key = "id";
    item_stack_t Item;
};
struct spectral_arrow_entity_format_t : arrow_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "spectral_arrow"; static constexpr std::string_view type_key = "id";
    std::optional<std::int32_t> Duration;
};
struct splash_potion_entity_format_t : generic_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "splash_potion"; static constexpr std::string_view type_key = "id";
    item_stack_t Item;
};
struct trident_entity_format_t : arrow_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "trident"; static constexpr std::string_view type_key = "id";
    bool DealtDamage = false;
};
struct wind_charge_entity_format_t : hurting_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "wind_charge"; static constexpr std::string_view type_key = "id";
};
struct wither_skull_entity_format_t : hurting_projectile_entity_format_t {
    static constexpr std::string_view type_tag = "wither_skull"; static constexpr std::string_view type_key = "id";
    bool dangerous;
};

struct container_entity_format_t {
    std::vector<item_stack_t> Items;
    std::optional<resource_location_t> LootTable;
    std::optional<std::int64_t> LootTableSeed;
};
struct generic_minecart_entity_format_t : generic_entity_format_t {
    std::optional<std::int32_t> DisplayOffset;
    std::optional<block_state_t> DisplayState;
};

struct boat_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "boat"; static constexpr std::string_view type_key = "id";
};
struct chest_boat_entity_format_t : generic_entity_format_t, container_entity_format_t {
    static constexpr std::string_view type_tag = "chest_boat"; static constexpr std::string_view type_key = "id";
};
struct minecart_entity_format_t : generic_minecart_entity_format_t {
    static constexpr std::string_view type_tag = "minecart"; static constexpr std::string_view type_key = "id";
};
struct chest_minecart_entity_format_t : generic_minecart_entity_format_t, container_entity_format_t {
    static constexpr std::string_view type_tag = "chest_minecart"; static constexpr std::string_view type_key = "id";
};
struct hopper_minecart_entity_format_t : generic_minecart_entity_format_t, container_entity_format_t {
    static constexpr std::string_view type_tag = "hopper_minecart"; static constexpr std::string_view type_key = "id";
    bool Enabled = true;
};
struct furnace_minecart_entity_format_t : generic_minecart_entity_format_t {
    static constexpr std::string_view type_tag = "furnace_minecart"; static constexpr std::string_view type_key = "id";
    short Fuel = 0;
    double PushX = 0;
    double PushZ = 0;
};
struct command_block_minecart_entity_format_t : generic_minecart_entity_format_t {
    static constexpr std::string_view type_tag = "command_block_minecart"; static constexpr std::string_view type_key = "id";
    std::string Command = "";
    std::string LastOutput = "";
    std::int32_t SuccessCount = 0;
    bool TrackOutput = true;
};
struct spawner_minecart_entity_format_t : generic_minecart_entity_format_t {
    static constexpr std::string_view type_tag = "spawner_minecart"; static constexpr std::string_view type_key = "id";
    short Delay = 0;
    std::optional<short> MaxNearbyEntities;
    std::optional<short> MaxSpawnDelay;
    std::optional<short> MinSpawnDelay;
    std::optional<short> RequiredPlayerRange;
    std::optional<short> SpawnCount;
    std::optional<std::shared_ptr<spawn_data_t>> SpawnData;
    std::optional<std::vector<spawn_potential_t>> SpawnPotentials;
    short SpawnRange = 4;
};
struct tnt_minecart_entity_format_t : generic_minecart_entity_format_t {
    static constexpr std::string_view type_tag = "tnt_minecart"; static constexpr std::string_view type_key = "id";
    std::int32_t fuse = -1;
    float explosion_power = 4.0f;
    float explosion_speed_factor = 1.0f;
};

struct experience_orb_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "experience_orb"; static constexpr std::string_view type_key = "id";
    short Age = 0;
    std::optional<std::int32_t> Count;
    std::optional<short> Health;
    std::optional<short> Value;
};
struct item_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "item"; static constexpr std::string_view type_key = "id";
    short Age = 0;
    std::optional<short> Health;
    item_stack_t Item;
    std::optional<std::array<std::int32_t, 4>> Owner;
    short PickupDelay = 0;
    std::optional<std::array<std::int32_t, 4>> Thrower;
};

struct falling_block_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "falling_block"; static constexpr std::string_view type_key = "id";
    block_state_t BlockState;
    std::optional<std::shared_ptr<wrapped_wrapped_block_entity_t>> TileEntityData;
    bool CancelDrop = false;
    bool DropItem = true;
    float FallHurtAmount = 0;
    std::int32_t FallHurtMax = 40;
    bool HurtEntities = false;
    std::int32_t Time = 0;
};
struct tnt_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "tnt"; static constexpr std::string_view type_key = "id";
    short fuse = 80;
    block_state_t block_state;
    float explosion_power = 4.0;
    std::optional<std::array<std::int32_t, 4>> owner;
};

struct display_brightness_t {
    std::int32_t block;
    std::int32_t sky;
};

struct display_rotation_axis_angle_t {
    float angle;
    std::array<float, 3> axis;
};
using display_rotation_t = std::variant<std::array<float, 4>, display_rotation_axis_angle_t>;

struct display_transformation_decomposed_t {
    display_rotation_t right_rotation;
    std::array<float, 3> scale;
    display_rotation_t left_rotation;
    std::array<float, 3> translation;
};
using display_transformation_t = std::variant<std::array<float, 16>, display_transformation_decomposed_t>;

struct display_entity_format_t : generic_entity_format_t {
    std::string billboard = "fixed"; // fixed || vertical || horizontal || center
    std::optional<display_brightness_t> brightness;
    std::optional<std::int32_t> glow_color_override; // -1 = team color
    float height = 0;
    float width = 0;
    std::optional<std::int32_t> interpolation_duration;
    std::optional<std::int32_t> teleport_duration; // clamped to 0..59
    std::optional<std::int32_t> start_interpolation; // not stored on entity; command-only
    float shadow_radius = 0;
    float shadow_strength = 1;
    float view_range = 1;
    std::optional<display_transformation_t> transformation;
};

struct block_display_entity_format_t {
    static constexpr std::string_view type_tag = "block_display"; static constexpr std::string_view type_key = "id";
    block_state_t block_state;
};
struct item_display_entity_format_t {
    static constexpr std::string_view type_tag = "item_display"; static constexpr std::string_view type_key = "id";
    std::optional<item_stack_t> item;
    std::string item_display = "none"; // none || thirdperson_lefthand || thirdperson_righthand || firstperson_lefthand || firstperson_righthand || head || gui || ground || fixed
};
struct text_display_entity_format_t {
    static constexpr std::string_view type_tag = "text_display"; static constexpr std::string_view type_key = "id";
    std::optional<text_t> text;
    std::string alignment = "center"; // center || left || right
    std::int32_t background = 1073741824;
    bool default_background = true;
    std::int32_t line_width = 200;
    bool see_through = true;
    bool shadow = false;
    std::int8_t text_opacity = -1;
};

struct area_effect_cloud_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "area_effect_cloud"; static constexpr std::string_view type_key = "id";

    std::int32_t Age = 0;
    std::int32_t Color;
    std::int32_t Duration;
    std::int32_t DurationOnUse;
    // TODO: potion_contents
    std::array<std::int32_t, 4> Owner;
    // TODO: custom_particle
    float potion_duration_scale;
    float Radius;
    float RadiusOnUse;
    float RadiusPerTick;
    std::int32_t ReapplicationDelay;
    std::int32_t WaitTime;
};
struct cushion_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "cushion"; static constexpr std::string_view type_key = "id";
    std::optional<std::array<std::int32_t, 3>> block_pos;
    std::string color = "white";
};
struct end_crystal_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "end_crystal"; static constexpr std::string_view type_key = "id";
    std::optional<std::array<std::int32_t, 3>> beam_target;
    bool ShowBottom = false;
};
struct evoker_fangs_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "evoker_fangs"; static constexpr std::string_view type_key = "id";
    std::optional<std::array<std::int32_t, 4>> Owner;
    std::int32_t Warmup = 0;
};
struct eye_of_ender_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "eye_of_ender"; static constexpr std::string_view type_key = "id";
    item_stack_t Item;
};
struct fishing_bobber_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "fishing_bobber"; static constexpr std::string_view type_key = "id";
};
struct glow_item_frame_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "glow_item_frame"; static constexpr std::string_view type_key = "id";
    std::int8_t Facing = 3; // 3 south, 4 west, 2 north, 5 east, 1 top, 0 bottom
    bool Fixed = false;
    bool Invisible = false;
    std::optional<item_stack_t> Item;
    float ItemDropChance = 1.0f;
    std::int8_t ItemRotation = 0;
};
struct interaction_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "interaction"; static constexpr std::string_view type_key = "id";

    float width = 1;
    float height = 1;
    bool response = false;
    struct interaction_t {
        std::array<std::int32_t, 4> player;
        std::int64_t timestamp;
    };
    std::optional<interaction_t> attack, interaction;
};
struct item_frame_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "item_frame"; static constexpr std::string_view type_key = "id";
    std::int8_t Facing = 3; // 3 south, 4 west, 2 north, 5 east, 1 top, 0 bottom
    bool Fixed = false;
    bool Invisible = false;
    std::optional<item_stack_t> Item;
    float ItemDropChance = 1.0f;
    std::int8_t ItemRotation = 0;
};
struct leash_knot_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "leash_knot"; static constexpr std::string_view type_key = "id";
};
struct lightning_bolt_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "lightning_bolt"; static constexpr std::string_view type_key = "id";
};
struct marker_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "marker"; static constexpr std::string_view type_key = "id";
};
struct ominous_item_spawner_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "ominous_item_spawner"; static constexpr std::string_view type_key = "id";
    item_stack_t item;
    std::optional<std::int64_t> spawn_item_after_ticks;
};
struct painting_entity_format_t : generic_entity_format_t {
    static constexpr std::string_view type_tag = "painting"; static constexpr std::string_view type_key = "id";
    std::int8_t facing = 0; // 0 south, 1 west, 2 north, 3 east
    resource_location_t variant;
};

// Entities discriminated by their "id" tag. Every alternative must share type_key "id" with a unique type_tag.
using typed_entity_format_t = std::variant<allay_mob_entity_format_t, armadillo_mob_entity_format_t, armor_stand_mob_entity_format_t,
    axolotl_mob_entity_format_t, bat_mob_entity_format_t, bee_mob_entity_format_t, blaze_mob_entity_format_t, bogged_mob_entity_format_t,
    breeze_mob_entity_format_t, camel_mob_entity_format_t, camel_husk_mob_entity_format_t, cat_mob_entity_format_t, chicken_mob_entity_format_t,
    cod_mob_entity_format_t, copper_golem_mob_entity_format_t, cow_mob_entity_format_t, creaking_mob_entity_format_t, creeper_mob_entity_format_t,
    dolphin_mob_entity_format_t, donkey_mob_entity_format_t, drowned_mob_entity_format_t, elder_guardian_mob_entity_format_t, ender_dragon_mob_entity_format_t,
    enderman_mob_entity_format_t, endermite_mob_entity_format_t, evoker_mob_entity_format_t, fox_mob_entity_format_t, frog_mob_entity_format_t,
    frostbite_mob_entity_format_t, ghast_mob_entity_format_t, giant_mob_entity_format_t, glow_squid_mob_entity_format_t, goat_mob_entity_format_t,
    guardian_mob_entity_format_t, happy_ghast_mob_entity_format_t, hoglin_mob_entity_format_t, husk_mob_entity_format_t,
    illusioner_mob_entity_format_t, iron_golem_mob_entity_format_t, llama_mob_entity_format_t, magma_cube_mob_entity_format_t, mannequin_mob_entity_format_t,
    mooshroom_mob_entity_format_t, mule_mob_entity_format_t, nautilus_mob_entity_format_t, ocelot_mob_entity_format_t, panda_mob_entity_format_t,
    parrot_mob_entity_format_t, phantom_mob_entity_format_t, pig_mob_entity_format_t, piglin_mob_entity_format_t, piglin_brute_mob_entity_format_t,
    pillager_mob_entity_format_t, player_mob_entity_format_t, polar_bear_mob_entity_format_t, pufferfish_mob_entity_format_t, rabbit_mob_entity_format_t,
    ravager_mob_entity_format_t, salmon_mob_entity_format_t, sheep_mob_entity_format_t, shulker_mob_entity_format_t, silverfish_mob_entity_format_t,
    skeleton_mob_entity_format_t, skeleton_horse_mob_entity_format_t, slime_mob_entity_format_t, sniffer_mob_entity_format_t, snow_golem_mob_entity_format_t,
    spider_mob_entity_format_t, strider_mob_entity_format_t, squid_mob_entity_format_t, stray_mob_entity_format_t, sulfur_cube_mob_entity_format_t,
    tadpole_mob_entity_format_t, trader_llama_mob_entity_format_t, tropical_fish_mob_entity_format_t, turtle_mob_entity_format_t, vex_mob_entity_format_t,
    villager_mob_entity_format_t, vindicator_mob_entity_format_t, wandering_trader_mob_entity_format_t, warden_mob_entity_format_t, witch_mob_entity_format_t,
    wither_mob_entity_format_t, wither_skeleton_mob_entity_format_t, wolf_mob_entity_format_t, zoglin_mob_entity_format_t, zombie_mob_entity_format_t,
    zombie_horse_mob_entity_format_t, zombie_nautilus_mob_entity_format_t, zombie_villager_mob_entity_format_t, zombified_piglin_mob_entity_format_t,
    arrow_entity_format_t, breeze_wind_charge_entity_format_t, dragon_fireball_entity_format_t, egg_entity_format_t, ender_pearl_entity_format_t,
    experience_bottle_entity_format_t, fireball_entity_format_t, firework_rocket_entity_format_t, lingering_potion_entity_format_t,
    llama_spit_entity_format_t, shulker_bullet_entity_format_t, small_fireball_entity_format_t, snowball_entity_format_t, spectral_arrow_entity_format_t,
    splash_potion_entity_format_t, trident_entity_format_t, wind_charge_entity_format_t, wither_skull_entity_format_t,
    boat_entity_format_t, chest_boat_entity_format_t, minecart_entity_format_t, chest_minecart_entity_format_t,
    hopper_minecart_entity_format_t, furnace_minecart_entity_format_t, command_block_minecart_entity_format_t, spawner_minecart_entity_format_t,
    tnt_minecart_entity_format_t, experience_orb_entity_format_t, item_entity_format_t, falling_block_entity_format_t, tnt_entity_format_t,
    area_effect_cloud_entity_format_t, cushion_entity_format_t, end_crystal_entity_format_t, evoker_fangs_entity_format_t,
    eye_of_ender_entity_format_t, fishing_bobber_entity_format_t, glow_item_frame_entity_format_t, interaction_entity_format_t,
    item_frame_entity_format_t, leash_knot_entity_format_t, lightning_bolt_entity_format_t, marker_entity_format_t, horse_mob_entity_format_t,
    ominous_item_spawner_entity_format_t, painting_entity_format_t, block_display_entity_format_t, item_display_entity_format_t, text_display_entity_format_t>;
using entity_format_t = std::variant<typed_entity_format_t, generic_mob_entity_format_t, generic_entity_format_t>;
struct wrapped_entity_format_t {
    entity_format_t value;

    // an untagged union: the first alternative that parses wins
    struct glaze {
        using R = wrapped_entity_format_t;
        static constexpr auto value = &R::value;
    };
};

struct entity_t {
    std::int32_t DataVersion;
    std::array<std::int32_t, 2> Position;
    std::vector<entity_format_t> Entities;
};
struct spawn_data_t {
    entity_format_t entity;
    struct custom_spawn_rules_t {
        std::int32_t block_light_limit;
        std::int32_t sky_light_limit;
    };
    std::optional<custom_spawn_rules_t> custom_spawn_rules;
    struct equipment_t {
        resource_location_t loot_table;
        std::variant<float, mob_drop_chances_t> slot_drop_chances;
    };
    std::optional<equipment_t> equipment;
};
struct spawn_potential_t {
    float weight;
    spawn_data_t data;
};

}
