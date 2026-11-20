/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/util/json_reflect.hpp>
#include <tungsten/world/item/item_stack.hpp>
#include "predicate.hpp"
#include "context_number_provider.hpp"

namespace tungsten {

struct item_modifier_t {
    std::vector<glz::generic> functions;
    std::optional<resource_location_t> reference;

    // a reference ("id"), a single function (object) or a list of functions
    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = item_modifier_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

struct loot_table_t {
    struct pool_t {
        std::optional<holder_t<predicate_t>> condition; // all conditions must pass
        std::variant<item_modifier_t, std::vector<item_modifier_t>> modifier = std::vector<item_modifier_t>(); // applied to every drop of the pool
        context_number_provider_t rolls;
        std::optional<context_number_provider_t> bonus_rolls;
        std::vector<glz::generic> entries = {}; // TODO! pool_entry_t
    };

    // minecraft:block || generic || entity || chest || ... ; absent in some vanilla tables used purely as functions
    resource_location_t type = resource_location_t("generic");
    std::variant<item_modifier_t, std::vector<item_modifier_t>> modifier = std::vector<item_modifier_t>(); // applied to every drop of the table
    std::vector<pool_t> pools = {};
    std::optional<resource_location_t> random_sequence;
};

}
