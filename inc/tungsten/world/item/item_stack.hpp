/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/resource/resource_location.hpp>
#include <tungsten/world/data_component.hpp>
#include <optional>

namespace tungsten {

struct item_stack_t {
    resource_location_t id;
    int count = 1;
    std::optional<data_components_t> components;
    std::optional<std::int16_t> Slot;
};

}
