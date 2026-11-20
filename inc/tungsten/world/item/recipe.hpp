/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <unordered_map>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <tungsten/util/json_reflect.hpp>
#include <tungsten/resource/resource_location.hpp>
#include <tungsten/resource/holder_set.hpp>
#include <tungsten/world/item/item_stack.hpp>

namespace tungsten {

// "type": "minecraft:crafting_shaped"
struct crafting_shaped_t {
    static constexpr std::string_view type_tag = "crafting_shaped";

    std::optional<std::string> group;
    std::string category = "misc"; // misc || equipment || building || redstone
    bool show_notification = true;
    std::unordered_map<std::string, holder_set_t> key;
    std::vector<std::string> pattern; // rows of the pattern
    item_stack_t result;
};

// "type": "minecraft:crafting_shapeless"
struct crafting_shapeless_t {
    static constexpr std::string_view type_tag = "crafting_shapeless";

    std::optional<std::string> group;
    std::string category = "misc"; // misc || equipment || building || redstone
    bool show_notification = true;
    std::vector<holder_set_t> ingredients;
    item_stack_t result;
};

// "type": "minecraft:crafting_dye"
struct crafting_dye_t {
    static constexpr std::string_view type_tag = "crafting_dye";

    std::optional<std::string> group;
    std::string category = "misc"; // misc || equipment || building || redstone
    bool show_notification = true;
    holder_set_t dye;
    holder_set_t target;
    item_stack_t result;
};

// "type": "minecraft:crafting_imbue"
struct crafting_imbue_t {
    static constexpr std::string_view type_tag = "crafting_imbue";

    std::optional<std::string> group;
    std::string category = "misc"; // misc || equipment || building || redstone
    bool show_notification = true;
    holder_set_t source;   // the ingredient in the center of the crafting grid
    holder_set_t material; // the ingredient in the eight other slots
    item_stack_t result;
};

// "type": "minecraft:crafting_transmute"
struct crafting_transmute_t {
    // material_count: an integer, or an inclusive range { min, max }
    struct material_count_range_t {
        int min;
        int max;
    };

    static constexpr std::string_view type_tag = "crafting_transmute";

    std::optional<std::string> group;
    std::string category = "misc"; // misc || equipment || building || redstone
    bool show_notification = true;
    holder_set_t input;
    holder_set_t material;
    std::optional<std::variant<int, material_count_range_t>> material_count;
    bool add_material_count_to_result = false;
    item_stack_t result;
};

// "type": "minecraft:crafting_special_*" (the special type is the serializer id, see special_type)
// Special recipes are built-in logic; their extra fields are kept verbatim in `extra`.
struct crafting_special_armordye {
    static constexpr std::string_view type_tag = "crafting_special_armordye";

    std::string special_type; // e.g. "crafting_special_armordye"
    std::string category = "misc"; // misc || equipment || building || redstone
};
struct crafting_special_bannerduplicate_t {
    static constexpr std::string_view type_tag = "crafting_special_bannerduplicate"; // placeholder, see recipe_t

    std::string special_type; // e.g. "crafting_special_armordye"
    std::string category = "misc"; // misc || equipment || building || redstone
    holder_set_t banner;
    item_stack_t result;
};
struct crafting_special_bookcloning_t {
    static constexpr std::string_view type_tag = "crafting_special_bookcloning"; // placeholder, see recipe_t

    std::string special_type; // e.g. "crafting_special_armordye"
    std::string category = "misc"; // misc || equipment || building || redstone
    holder_set_t source;
    holder_set_t material;
    std::array<std::int32_t, 2> allowed_generations = { 0, 1 };
    item_stack_t result;
};
struct crafting_special_firework_rocket_t {
    static constexpr std::string_view type_tag = "crafting_special_firework_rocket"; // placeholder, see recipe_t

    std::string special_type; // e.g. "crafting_special_armordye"
    std::string category = "misc"; // misc || equipment || building || redstone
    holder_set_t fuel;
    holder_set_t shell;
    holder_set_t star;
    item_stack_t result;
};
struct crafting_special_firework_star_t {
    static constexpr std::string_view type_tag = "crafting_special_firework_star"; // placeholder, see recipe_t

    std::string special_type; // e.g. "crafting_special_armordye"
    std::string category = "misc"; // misc || equipment || building || redstone
    holder_set_t dye;
    holder_set_t fuel;
    std::unordered_map<std::string, holder_set_t> shapes; // shape name -> holder set
    holder_set_t trail;
    holder_set_t twinkle;
    item_stack_t result;
};
struct crafting_special_firework_star_fade_t {
    static constexpr std::string_view type_tag = "crafting_special_firework_star_fade"; // placeholder, see recipe_t

    std::string special_type; // e.g. "crafting_special_armordye"
    std::string category = "misc"; // misc || equipment || building || redstone
    holder_set_t dye;
    holder_set_t target;
    item_stack_t result;
};
struct crafting_special_mapcloning_t {
    static constexpr std::string_view type_tag = "crafting_special_mapcloning"; // placeholder, see recipe_t

    std::string special_type; // e.g. "crafting_special_armordye"
    std::string category = "misc"; // misc || equipment || building || redstone
};
struct crafting_special_mapextending_t {
    static constexpr std::string_view type_tag = "crafting_special_mapextending"; // placeholder, see recipe_t

    std::string special_type; // e.g. "crafting_special_armordye"
    std::string category = "misc"; // misc || equipment || building || redstone
    holder_set_t map;
    holder_set_t material;
    item_stack_t result;
};
struct crafting_special_repairitem_t {
    static constexpr std::string_view type_tag = "crafting_special_repairitem"; // placeholder, see recipe_t

    std::string special_type; // e.g. "crafting_special_armordye"
    std::string category = "misc"; // misc || equipment || building || redstone
};
struct crafting_special_shielddecoration_t {
    static constexpr std::string_view type_tag = "crafting_special_shielddecoration"; // placeholder, see recipe_t

    std::string special_type; // e.g. "crafting_special_armordye"
    std::string category = "misc"; // misc || equipment || building || redstone
    holder_set_t banner;
    item_stack_t result;
    holder_set_t target;
};
struct crafting_special_tippedarrow_t {
    static constexpr std::string_view type_tag = "crafting_special_tippedarrow"; // placeholder, see recipe_t

    std::string special_type; // e.g. "crafting_special_armordye"
    std::string category = "misc"; // misc || equipment || building || redstone
};

// "type": "minecraft:crafting_decorated_pot"
struct crafting_decorated_pot_t {
    static constexpr std::string_view type_tag = "crafting_decorated_pot";

    std::string category = "misc"; // misc || equipment || building || redstone
    holder_set_t back;
    holder_set_t front;
    holder_set_t left;
    holder_set_t right;
    item_stack_t result;
};

// "type": "minecraft:smelting"
struct smelting_t {
    static constexpr std::string_view type_tag = "smelting";

    std::optional<resource_location_t> group;
    std::string category = "misc"; // blocks || food || misc
    bool show_notification = true;
    holder_set_t ingredient;
    int cookingtime = 200;
    item_stack_t result;
    std::optional<double> experience;
};

// "type": "minecraft:blasting"
struct blasting_t {
    static constexpr std::string_view type_tag = "blasting";

    std::optional<resource_location_t> group;
    std::string category = "misc"; // blocks || misc
    bool show_notification = true;
    holder_set_t ingredient;
    int cookingtime = 100;
    item_stack_t result;
    std::optional<double> experience;
};

// "type": "minecraft:smoking"
struct smoking_t {
    static constexpr std::string_view type_tag = "smoking";

    std::optional<resource_location_t> group;
    std::string category = "food"; // food
    bool show_notification = true;
    holder_set_t ingredient;
    int cookingtime = 100;
    item_stack_t result;
    std::optional<double> experience;
};

// "type": "minecraft:campfire_cooking"
struct campfire_cooking_t {
    static constexpr std::string_view type_tag = "campfire_cooking";

    std::optional<resource_location_t> group;
    std::string category = "food"; // food
    bool show_notification = true;
    holder_set_t ingredient;
    int cookingtime = 100;
    item_stack_t result;
    std::optional<double> experience;
};

// "type": "minecraft:stonecutting"
struct stonecutting_t {
    static constexpr std::string_view type_tag = "stonecutting";

    bool show_notification = true;
    holder_set_t ingredient;
    item_stack_t result;
};

// "type": "minecraft:smithing_transform"
struct smithing_transform_t {
    static constexpr std::string_view type_tag = "smithing_transform";

    bool show_notification = true;
    std::optional<holder_set_t> template_; // "template" is a keyword
    holder_set_t base;
    std::optional<holder_set_t> addition;
    item_stack_t result;
};

// "type": "minecraft:smithing_trim"
struct smithing_trim_t {
    static constexpr std::string_view type_tag = "smithing_trim";

    bool show_notification = true;
    std::optional<holder_set_t> template_;
    holder_set_t base;
    std::optional<holder_set_t> addition;
    holder_set_t pattern;
};

// "type": "minecraft:brewing"
struct brewing_ingredient_t { // TODO
    resource_location_t item;
    std::optional<glz::generic> potion_contents; // a minecraft:potion_contents data component predicate
};
struct brewing_t {
    static constexpr std::string_view type_tag = "brewing";

    brewing_ingredient_t input;
    brewing_ingredient_t reagent;
    item_stack_t output;
};

// one variant per "type" (tag "minecraft:..." and bare ids both match)
using recipe_t = std::variant<
    crafting_shaped_t,
    crafting_shapeless_t,
    crafting_dye_t,
    crafting_imbue_t,
    crafting_transmute_t,
    crafting_decorated_pot_t,
    smelting_t,
    blasting_t,
    smoking_t,
    campfire_cooking_t,
    stonecutting_t,
    smithing_transform_t,
    smithing_trim_t,
    crafting_special_armordye,
    crafting_special_bannerduplicate_t,
    crafting_special_bookcloning_t,
    crafting_special_firework_rocket_t,
    crafting_special_firework_star_t,
    crafting_special_firework_star_fade_t,
    crafting_special_mapcloning_t,
    crafting_special_mapextending_t,
    crafting_special_repairitem_t,
    crafting_special_shielddecoration_t,
    crafting_special_tippedarrow_t,
    brewing_t>;

}

// members named like a C++ keyword carry a trailing underscore which is not part of the key
template <> struct glz::meta<tungsten::smithing_transform_t> {
    static constexpr std::string_view rename_key(const std::string_view key) { return key.ends_with('_') ? key.substr(0, key.size() - 1) : key; }
};

template <> struct glz::meta<tungsten::smithing_trim_t> {
    static constexpr std::string_view rename_key(const std::string_view key) { return key.ends_with('_') ? key.substr(0, key.size() - 1) : key; }
};
