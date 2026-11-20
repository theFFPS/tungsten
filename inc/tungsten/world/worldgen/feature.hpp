/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/util/json_reflect.hpp>
#include <tungsten/resource/holder.hpp>
#include <tungsten/type/typed_json.hpp>

namespace tungsten {

struct configured_feature_t {
    typed_json_t def; // "type" + config members are all in the same object (flattened in 26.x)

    struct glaze {
        using R = configured_feature_t;
        static constexpr auto value = &R::def;
    };
};
struct placed_feature_t {
    holder_t<configured_feature_t> feature;
    std::vector<typed_json_t> placement = {}; // placement modifiers
};

}
