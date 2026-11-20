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
#include <tungsten/type/text_component.hpp>

namespace tungsten {

struct advancement_t {
    struct display_t {
        item_stack_t icon;
        text_t title;
        text_t description;
        std::string frame = "task"; // task || challenge || goal
        std::optional<resource_location_t> background;
        bool show_toast = true;
        bool announce_to_chat = true;
        bool hidden = false;
    };
    struct criterion_t {
        resource_location_t trigger;
        std::optional<glz::generic> conditions; // TODO! handle conditions
    };
    struct rewards_t {
        int experience = 0;
        std::vector<resource_location_t> recipes = {};
        std::vector<resource_location_t> loot = {};
        std::optional<resource_location_t> function;
    };

    std::optional<resource_location_t> parent;
    std::optional<display_t> display;
    std::map<std::string, criterion_t> criteria;
    std::vector<std::vector<std::string>> requirements = {};
    std::optional<rewards_t> rewards;
    bool sends_telemetry_event = false;

    void post_load();
};

}
