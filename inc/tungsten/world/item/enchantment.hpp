/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/util/json_reflect.hpp>
#include <tungsten/type/int_provider.hpp>
#include <tungsten/type/text_component.hpp>
#include <tungsten/resource/holder_set.hpp>

namespace tungsten {

struct enchantment_t {
    // TODO! implement effect components

    struct cost_t {
        int base;
        int per_level_above_first;
    };

    text_t description;
    holder_set_t exclusive_set = {};
    holder_set_t supported_items;
    std::optional<holder_set_t> primary_items;
    float weight;
    int max_level;
    cost_t min_cost;
    cost_t max_cost;
    int anvil_cost;
    std::vector<std::string> slots; // any || hand || mainhand || offhand || armor || feet || legs || chest || head || body || saddle
    // enchantment effect components: type id -> definition (list or object)
    std::map<resource_location_t, glz::generic> effects = {};
};

struct enchantment_provider_t {
    resource_location_t type; // single || by_cost || by_cost_with_difficulty
    // single
    std::optional<resource_location_t> enchantment;
    std::optional<int_provider_t> level; // int provider
    // by_cost, by_cost_with_difficulty
    std::optional<holder_set_t> enchantments;
    std::optional<int_provider_t> cost; // by_cost: int provider
    std::optional<int> min_cost;
    std::optional<int> max_cost_span;
    std::map<std::string, glz::generic> extra = {}; // type specific keys which are not modelled

    struct glaze {
        using R = enchantment_provider_t;
        static constexpr auto value = glz::object("type", &R::type, "enchantment", &R::enchantment, "level", &R::level,
            "enchantments", &R::enchantments, "cost", &R::cost, "min_cost", &R::min_cost, "max_cost_span", &R::max_cost_span);
        static constexpr auto unknown_read = &R::extra;
        static constexpr auto unknown_write = &R::extra;
    };
};

}
