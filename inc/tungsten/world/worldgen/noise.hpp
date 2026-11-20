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
#include <tungsten/resource/holder_set.hpp>
#include <tungsten/type/typed_json.hpp>
#include <tungsten/type/int_provider.hpp>
#include <tungsten/world/block/block_state.hpp>
#include "biome.hpp"

namespace tungsten {

struct wrapped_density_function_t;

struct density_function__cache_t {
    static constexpr std::string_view type_tag = "cache";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
};
struct density_function__interpolated_t {
    static constexpr std::string_view type_tag = "interpolated";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
    std::int32_t cell_size_xz;
    std::int32_t cell_size_y;
};
struct density_function__upscale_t {
    static constexpr std::string_view type_tag = "upscale";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
    struct point_t {
        std::string mode;
        int cell_size;
    };

    point_t x;
    point_t y;
    point_t z;
};
struct density_function__noise_t {
    static constexpr std::string_view type_tag = "noise";
    resource_location_t noise;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> xz_scale;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> y_scale;
    std::optional<std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>>> shift_x;
    std::optional<std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>>> shift_y;
    std::optional<std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>>> shift_z;
};
struct density_function__old_blended_noise_t {
    static constexpr std::string_view type_tag = "old_blended_noise";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> xz_scale;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> y_scale;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> xz_factor;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> y_factor;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> smear_scale_multiplier;
};
struct density_function__shift_t {
    static constexpr std::string_view type_tag = "shift";
    resource_location_t noise;
};
struct density_function__shift_a_t {
    static constexpr std::string_view type_tag = "shift_a";
    resource_location_t noise;
};
struct density_function__shift_b_t {
    static constexpr std::string_view type_tag = "shift_b";
    resource_location_t noise;
};
struct density_function__abs_t {
    static constexpr std::string_view type_tag = "abs";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
};
struct density_function__add_t {
    static constexpr std::string_view type_tag = "add";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> left;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> right;
};
struct density_function__ceil_t {
    static constexpr std::string_view type_tag = "ceil";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> multiple;
};
struct density_function__clamp_t {
    static constexpr std::string_view type_tag = "clamp";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
    std::optional<double> min;
    std::optional<double> max;
};
struct density_function__constant_t {
    static constexpr std::string_view type_tag = "constant";
    double argument;
};
struct density_function__cube_t {
    static constexpr std::string_view type_tag = "cube";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
};
struct density_function__div_t {
    static constexpr std::string_view type_tag = "div";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> left;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> right;
};
struct density_function__floor_t {
    static constexpr std::string_view type_tag = "floor";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> multiple;
};
struct density_function__log_t {
    static constexpr std::string_view type_tag = "log";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
};
struct density_function__mul_t {
    static constexpr std::string_view type_tag = "mul";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> left;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> right;
};
struct density_function__min_t {
    static constexpr std::string_view type_tag = "min";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> left;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> right;
};
struct density_function__max_t {
    static constexpr std::string_view type_tag = "max";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> left;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> right;
};
struct density_function__negate_t {
    static constexpr std::string_view type_tag = "negate";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
};
struct density_function__pow_t {
    static constexpr std::string_view type_tag = "pow";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> base;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> exponent;
};
struct density_function__reciprocal_t {
    static constexpr std::string_view type_tag = "reciprocal";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> argument;
};
struct density_function__round_t {
    static constexpr std::string_view type_tag = "round";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> multiple;
};
struct density_function__sub_t {
    static constexpr std::string_view type_tag = "sub";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> left;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> right;
};
struct density_function__sign_t {
    static constexpr std::string_view type_tag = "sign";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
};
struct density_function__sqrt_t {
    static constexpr std::string_view type_tag = "sqrt";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
};
struct density_function__square_t {
    static constexpr std::string_view type_tag = "square";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
};
struct density_function__truncate_t {
    static constexpr std::string_view type_tag = "truncate";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> multiple;
};
struct density_function__gradient_t {
    static constexpr std::string_view type_tag = "gradient";
    std::string axis; // x || y || z
    std::string tiling = "clamp_to_edge"; // clamp_to_edge || repeat || mirrored_repeat
    std::optional<std::int32_t> from_coordinate;
    std::optional<std::int32_t> to_coordinate;
    float from_value;
    float to_value;
};
struct density_function__lerp_t {
    static constexpr std::string_view type_tag = "lerp";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> alpha;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> first;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> second;
};
struct density_function__squeeze_t {
    static constexpr std::string_view type_tag = "squeeze";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
};
struct density_function__spline_t {
    static constexpr std::string_view type_tag = "spline";

    struct spline_t {
        std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> coordinate;

        struct point_t {
            float location;
            std::variant<float, std::shared_ptr<spline_t>> value;
            float derivative;
        };
        std::vector<point_t> points;
    };

    std::variant<float, spline_t> spline;
};
struct density_function__half_negative_t {
    static constexpr std::string_view type_tag = "half_negative";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
};
struct density_function__interval_select_t {
    static constexpr std::string_view type_tag = "interval_select";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
    std::vector<float> thresholds;
    std::vector<std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>>> functions;
};
struct density_function__quarter_negative_t {
    static constexpr std::string_view type_tag = "quarter_negative";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
};
struct density_function__range_choice_t {
    static constexpr std::string_view type_tag = "range_choice";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
    std::optional<double> min_inclusive;
    std::optional<double> max_exclusive;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> when_in_range;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> when_out_of_range;
};
struct density_function__beardifier_t {
    static constexpr std::string_view type_tag = "beardifier";
};
struct density_function__blend_alpha_t {
    static constexpr std::string_view type_tag = "blend_alpha";
};
struct density_function__blend_density_t {
    static constexpr std::string_view type_tag = "blend_density";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
};
struct density_function__blend_offset_t {
    static constexpr std::string_view type_tag = "blend_offset";
};
struct density_function__distance_to_point_t {
    static constexpr std::string_view type_tag = "distance_to_point";
    std::array<std::int32_t, 3> point;
    std::string metric;
    /*
     * euclidean: sqrt(dx^2 + dy^2 + dz^2)
     * euclidean_squared: dx^2 + dy^2 + dz^2
     * manhattan: abs(dx) + abs(dy) + abs(dz)
     * chebyshev: max(abs(dx), abs(dy), abs(dz))
     */
};
struct density_function__distance_warp_t {
    static constexpr std::string_view type_tag = "distance_warp";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> offset_x;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> offset_y;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> offset_z;
};
struct density_function__end_outer_islands_t {
    static constexpr std::string_view type_tag = "end_outer_islands";
};
struct density_function__find_top_surface_t {
    static constexpr std::string_view type_tag = "find_top_surface";
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> density;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> upper_bound;
    std::int32_t lower_bound;
    std::int32_t cell_height;
};
struct density_function__ore_vein_t {
    static constexpr std::string_view type_tag = "ore_vein";
    resource_location_t ore_block;
    resource_location_t raw_ore_block;
    resource_location_t filler_block;
    float raw_ore_chance;
    std::int32_t density;
    std::int32_t richness;
    std::int32_t filler_gap;
};
struct density_function__slice_t {
    static constexpr std::string_view type_tag = "slice";
    std::string axis; // x || y || z
    std::int32_t coordinate;
    std::variant<resource_location_t, std::shared_ptr<wrapped_density_function_t>> input;
};

using typed_density_function_t = std::variant<density_function__cache_t, density_function__interpolated_t, density_function__upscale_t, density_function__noise_t,
    density_function__old_blended_noise_t, density_function__shift_t, density_function__shift_a_t, density_function__shift_b_t, density_function__abs_t,
    density_function__add_t, density_function__ceil_t, density_function__clamp_t, density_function__constant_t, density_function__cube_t, density_function__div_t,
    density_function__floor_t, density_function__log_t, density_function__mul_t, density_function__min_t, density_function__max_t, density_function__negate_t,
    density_function__pow_t, density_function__reciprocal_t, density_function__round_t, density_function__sub_t, density_function__sign_t, density_function__sqrt_t,
    density_function__square_t, density_function__truncate_t, density_function__gradient_t, density_function__lerp_t, density_function__squeeze_t,
    density_function__spline_t, density_function__half_negative_t, density_function__interval_select_t, density_function__quarter_negative_t,
    density_function__range_choice_t, density_function__beardifier_t, density_function__blend_alpha_t, density_function__blend_density_t,
    density_function__blend_offset_t, density_function__distance_to_point_t, density_function__distance_warp_t, density_function__end_outer_islands_t,
    density_function__find_top_surface_t, density_function__ore_vein_t, density_function__slice_t>;
using density_function_t = std::variant<typed_density_function_t, double>;
struct wrapped_density_function_t {
    density_function_t value;

    struct glaze {
        using R = wrapped_density_function_t;
        static constexpr auto value = &R::value;
    };
};

using noise_router_t = std::unordered_map<std::string, holder_t<density_function_t>>;

struct wrapped_material_rule_t;

struct wrapped_material_condition_t;

struct material_condition__above_preliminary_surface_t {
    static constexpr std::string_view type_tag = "above_preliminary_surface";
};
struct material_condition__biome_t {
    static constexpr std::string_view type_tag = "biome";
    std::vector<resource_location_t> biome_is;
};
struct material_condition__hole_t {
    static constexpr std::string_view type_tag = "hole";
};
struct material_condition__noise_threshold_t {
    static constexpr std::string_view type_tag = "noise_threshold";
    resource_location_t noise;
    std::optional<double> min_threshold;
    std::optional<double> max_threshold;
};
struct material_condition__not_t {
    static constexpr std::string_view type_tag = "not";
    std::shared_ptr<wrapped_material_condition_t> invert;
};
struct material_condition__steep_t {
    static constexpr std::string_view type_tag = "steep";
};
struct material_condition__stone_depth_t {
    static constexpr std::string_view type_tag = "stone_depth";
    std::string surface_type; // floor || ceiling
    std::int32_t offset;
    bool add_surface_depth;
    std::int32_t secondary_depth_range;
};
struct material_condition__temperature_t {
    static constexpr std::string_view type_tag = "temperature";
};
struct material_condition__vertical_gradient_t {
    static constexpr std::string_view type_tag = "vertical_gradient";
    std::string random_name;
    vertical_anchor_t true_at_and_below;
    vertical_anchor_t false_at_and_above;
};
struct material_condition__water_t {
    static constexpr std::string_view type_tag = "water";
    std::int32_t offset;
    std::int32_t surface_depth_multiplier;
    bool add_stone_depth;
};
struct material_condition__y_above_t {
    static constexpr std::string_view type_tag = "y_above";
    vertical_anchor_t anchor;
    std::int32_t surface_depth_multiplier;
    bool add_stone_depth;
};

using material_condition_t = std::variant<material_condition__above_preliminary_surface_t, material_condition__biome_t, material_condition__hole_t,
    material_condition__noise_threshold_t, material_condition__not_t, material_condition__steep_t, material_condition__stone_depth_t,
    material_condition__temperature_t, material_condition__vertical_gradient_t, material_condition__water_t, material_condition__y_above_t>;
struct wrapped_material_condition_t {
    material_condition_t value;

    struct glaze {
        using R = wrapped_material_condition_t;
        static constexpr auto value = &R::value;
    };
};

struct material_rule__badlands_t {
    static constexpr std::string_view type_tag = "badlands";
};
struct material_rule__block_t {
    static constexpr std::string_view type_tag = "block";
    std::shared_ptr<block_state_t> result_state;
};
struct material_rule__condition_t {
    static constexpr std::string_view type_tag = "condition";
    material_condition_t if_true;
    std::shared_ptr<wrapped_material_rule_t> then_run;
};
struct material_rule__sequence_t {
    static constexpr std::string_view type_tag = "sequence";
    std::vector<wrapped_material_rule_t> sequence;
};

using material_rule_t = std::variant<material_rule__badlands_t, material_rule__block_t, material_rule__condition_t, material_rule__sequence_t>;

struct wrapped_material_rule_t {
    material_rule_t value;

    struct glaze {
        using R = wrapped_material_rule_t;
        static constexpr auto value = &R::value;
    };
};

struct noise_t {
    int base_octave;
    std::vector<double> amplitude_modifiers = {};
    float base_amplitude;
    std::optional<int> octave_count;
    bool normalize = true;
};
struct noise_settings_t {
    struct debug_function_t {
        std::string label;
        holder_t<density_function_t> function;
    };

    int sea_level;
    bool disable_mob_generation = false;
    std::optional<bool> aquifers_enabled;
    bool legacy_random_source = false;
    std::vector<debug_function_t> debug_functions = {};
    resource_location_t default_block; // block id; legacy form is a block state { Name, Properties } and is not supported
    resource_location_t default_fluid;
    std::vector<biome_noise_parameters_t> spawn_target;
    struct noise_fields_t {
        int min_y;
        int height;
        std::optional<int> size_horizontal;
        std::optional<int> size_vertical;
    } noise;
    std::optional<noise_router_t> aquifers; // optional section in this version
    noise_router_t noise_router; // map: router entry -> density function (id / constant / inline)
    resource_location_t material_rule; // reference to a worldgen/material_rule
};

}
