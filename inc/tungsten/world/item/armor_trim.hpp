/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/resource/resource_location.hpp>
#include <tungsten/type/text_component.hpp>

namespace tungsten {

struct trim_material_t {
    resource_location_t palette_id;
    text_t description;
};

struct trim_pattern_t {
    resource_location_t asset_id;
    text_t description;
    bool decal = false;
};

}
