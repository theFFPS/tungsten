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
#include <tungsten/type/float_provider.hpp>

namespace tungsten {

struct configured_carver__cave_t {
    static constexpr std::string_view type_tag = "cave";
    float probability;
    height_provider_t y;
    float_provider_t room_vertical_radius_multiplier;
    height_provider_t count;
    float_provider_t horizontal_radius_multiplier;
    float_provider_t vertical_radius_multiplier;
    float_provider_t start_vertical_radius_multiplier = 1.0f;
    float_provider_t floor_level;
    float_provider_t thickness;
    bool weird_thickness_bias = false;
};
struct configured_carver__canyon_t {
    static constexpr std::string_view type_tag = "canyon";
    float probability;
    height_provider_t y;
    float_provider_t vertical_rotation;
    struct shape_t {
        float_provider_t y_scale;
        float_provider_t distance_factor;
        float_provider_t thickness;
        float_provider_t horizontal_radius_factor;
        float vertical_radius_default_factor;
        float vertical_radius_center_factor;
        int width_smoothness;
    } shape;
};
using configured_carver_t = std::variant<configured_carver__cave_t, configured_carver__canyon_t>;

}
