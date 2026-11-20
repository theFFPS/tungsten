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

struct float_provider__constant_t {
    static constexpr std::string_view type_tag = "constant";
    float value;
};
struct float_provider__uniform_t {
    static constexpr std::string_view type_tag = "uniform";
    std::optional<float> min_inclusive;
    std::optional<float> max_exclusive;
};
struct float_provider__clamped_normal_t {
    static constexpr std::string_view type_tag = "clamped_normal";
    float mean;
    float deviation;
    std::optional<float> min;
    std::optional<float> max;
};
struct float_provider__trapezoid_t {
    static constexpr std::string_view type_tag = "trapezoid";
    std::optional<float> min;
    std::optional<float> max;
    float plateau;
};
using typed_float_provider_t = std::variant<float_provider__constant_t, float_provider__uniform_t, float_provider__clamped_normal_t, float_provider__trapezoid_t>;
using float_provider_t = std::variant<typed_float_provider_t, float>;

}
