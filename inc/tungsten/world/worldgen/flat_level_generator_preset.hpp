/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <optional>
#include <vector>
#include <tungsten/resource/holder_set.hpp>
#include <tungsten/resource/resource_location.hpp>
#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

struct flat_generation_settings_t {
    struct layer_t {
        int height;
        resource_location_t block = resource_location_t("air");
    };

    std::vector<layer_t> layers;
    resource_location_t biome = resource_location_t("plains");
    bool lakes = false;
    bool features = false;
    std::optional<holder_set_t> structure_overrides;
};

struct flat_level_generator_preset_t {
    resource_location_t display;
    flat_generation_settings_t settings;
};

}
