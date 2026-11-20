/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/util/json_reflect.hpp>
#include <tungsten/resource/resource_location.hpp>
#include <tungsten/resource/holder_set.hpp>
#include <tungsten/world/data_component.hpp>
#include <tungsten/world/item/item_stack.hpp>
#include <tungsten/world/context_number_provider.hpp>
#include <tungsten/world/predicate.hpp>

namespace tungsten {

struct trade_item_t {
    resource_location_t id;
    context_number_provider_t count = 1;
    std::optional<data_components_t> components;
};
struct villager_trade_t {
    trade_item_t wants;
    std::optional<trade_item_t> additional_wants;
    item_stack_t gives;
    std::optional<std::variant<std::vector<glz::generic>, glz::generic>> given_item_modifier; // item modifier(s) TODO!
    context_number_provider_t max_uses = 4;
    context_number_provider_t reputation_discount = 0.0F;
    context_number_provider_t xp = 1;
    std::optional<predicate_t> merchant_predicate;
    std::optional<holder_set_t> double_trade_price_enchantments;
};
struct trade_set_t {
    holder_set_t trades;
    context_number_provider_t amount;
    bool allow_duplicates = false;
    std::optional<resource_location_t> random_sequence;
};

}
