/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/resource/resource_location.hpp>
#include <tungsten/world/entity/entity.hpp>
#include <vector>

namespace tungsten {

struct trial_spawner_t {
    struct loot_entry_t {
        resource_location_t data;
        int weight = 1;
    };

    int spawn_range = 4;
    float total_mobs = 6;
    float simultaneous_mobs = 2;
    float total_mobs_added_per_player = 2;
    float simultaneous_mobs_added_per_player = 1;
    int ticks_between_spawn = 40;
    std::vector<spawn_potential_t> spawn_potentials = {};
    std::vector<loot_entry_t> loot_tables_to_eject = {
        loot_entry_t{
            .data = resource_location_t("spawners/trial_chamber/consumables"),
            .weight = 1
        },
        loot_entry_t{
            .data = resource_location_t("spawners/trial_chamber/key"),
            .weight = 1
        }
    };
    resource_location_t items_to_drop_when_ominous = resource_location_t("spawners/trial_chamber/items_to_drop_when_ominous");
};

}
