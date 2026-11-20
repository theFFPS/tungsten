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
#include <tungsten/type/variant.hpp>

namespace tungsten {

struct vertical_anchor_t {
    std::optional<int> absolute;
    std::optional<int> above_bottom;
    std::optional<int> below_top;
};

struct wrapped_height_provider_t;
struct height_provider__constant_t {
    static constexpr std::string_view type_tag = "constant";
    vertical_anchor_t value;
};
struct height_provider__uniform_t {
    static constexpr std::string_view type_tag = "uniform";
    std::optional<vertical_anchor_t> min_inclusive;
    std::optional<vertical_anchor_t> max_inclusive;
};
struct height_provider__biased_to_bottom_t {
    static constexpr std::string_view type_tag = "biased_to_bottom";
    std::optional<vertical_anchor_t> min_inclusive;
    std::optional<vertical_anchor_t> max_inclusive;
    int inner = 1;
};
struct height_provider__very_biased_to_bottom_t {
    static constexpr std::string_view type_tag = "very_biased_to_bottom";
    std::optional<vertical_anchor_t> min_inclusive;
    std::optional<vertical_anchor_t> max_inclusive;
    int inner = 1;
};
struct height_provider__trapezoid_t {
    static constexpr std::string_view type_tag = "trapezoid";
    std::optional<vertical_anchor_t> min_inclusive;
    std::optional<vertical_anchor_t> max_inclusive;
    int plateau = 0;
};
struct height_provider__weighted_list_t {
    static constexpr std::string_view type_tag = "weighted_list";

    struct entry_t {
        int weight;
        std::shared_ptr<wrapped_height_provider_t> data;
    };

    std::vector<entry_t> distribution;
};

using typed_height_provider_t = std::variant<height_provider__constant_t, height_provider__uniform_t, height_provider__biased_to_bottom_t,
    height_provider__very_biased_to_bottom_t, height_provider__trapezoid_t, height_provider__weighted_list_t>;
using height_provider_t = std::variant<typed_height_provider_t, vertical_anchor_t, int>;
struct wrapped_height_provider_t {
    height_provider_t value;

    struct glaze {
        using R = wrapped_height_provider_t;
        static constexpr auto value = &R::value;
    };
};

struct int_provider__constant_t {
    static constexpr std::string_view type_tag = "constant";
    int value;
};
struct int_provider__uniform_t {
    static constexpr std::string_view type_tag = "uniform";
    int min_inclusive;
    int max_inclusive;
};
struct int_provider__biased_to_bottom_t {
    static constexpr std::string_view type_tag = "biased_to_bottom";
    int min_inclusive;
    int max_inclusive;
};
struct int_provider__clamped_normal_t {
    static constexpr std::string_view type_tag = "clamped_normal";
    float mean;
    float deviation;
    int min_inclusive;
    int max_inclusive;
};
struct int_provider__clamped_t;
struct int_provider__weighted_list_t;

using complex_int_provider_t = std::variant<int_provider__constant_t, int_provider__uniform_t, int_provider__biased_to_bottom_t,
    int_provider__clamped_normal_t, std::shared_ptr<int_provider__clamped_t>, std::shared_ptr<int_provider__weighted_list_t>>;
using int_provider_t = std::variant<int, complex_int_provider_t>;

struct int_provider__clamped_t {
    static constexpr std::string_view type_tag = "clamped";
    int min_inclusive;
    int max_inclusive;
    int_provider_t source;
};
struct int_provider__weighted_list_t {
    struct entry_t {
        float weight;
        int_provider_t data;
    };

    static constexpr std::string_view type_tag = "weighted_list";
    std::vector<entry_t> distribution;
};

}
