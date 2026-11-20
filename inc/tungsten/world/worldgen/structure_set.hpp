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
#include <tungsten/type/typed_json.hpp>

namespace tungsten {

struct structure_set_t {
    struct entry_t {
        resource_location_t structure;
        float weight;
    };

    std::vector<entry_t> structures;
    typed_json_t placement; // random_spread || concentric_rings
};

}
