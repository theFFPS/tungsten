/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

struct level_based_value__exponent_t;
struct level_based_value__clamped_t;
struct level_based_value__fraction_t;
struct level_based_value__lookup_t;
struct level_based_value__linear_t {
    static constexpr std::string_view type_tag = "linear";
    float base;
    float per_level_above_first;
};
struct level_based_value__levels_squared_t {
    static constexpr std::string_view type_tag = "levels_squared";
    std::optional<float> added;
};
using complex_level_based_value_t = std::variant<level_based_value__linear_t, level_based_value__levels_squared_t,
    std::shared_ptr<level_based_value__exponent_t>, std::shared_ptr<level_based_value__clamped_t>,
    std::shared_ptr<level_based_value__fraction_t>, std::shared_ptr<level_based_value__lookup_t>>;
using level_based_value_t = std::variant<float, complex_level_based_value_t>;
struct level_based_value__exponent_t {
    static constexpr std::string_view type_tag = "exponent";
    level_based_value_t base;
    level_based_value_t power;
};
struct level_based_value__clamped_t {
    static constexpr std::string_view type_tag = "clamped";
    level_based_value_t value;
    std::optional<float> min;
    std::optional<float> max;
};
struct level_based_value__fraction_t {
    static constexpr std::string_view type_tag = "fraction";
    level_based_value_t numerator;
    level_based_value_t denominator;
};
struct level_based_value__lookup_t {
    static constexpr std::string_view type_tag = "lookup";
    std::vector<float> values;
    std::optional<level_based_value_t> fallback;
};

}
