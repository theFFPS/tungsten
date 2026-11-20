/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/world/predicate.hpp>
#include <tungsten/world/block/block_state.hpp>
#include <tungsten/world/worldgen/noise.hpp>
#include <tungsten/type/int_provider.hpp>
#include <tungsten/util/json_range.hpp>
#include <tungsten/world/predicate.hpp>

namespace tungsten {

struct wrapped_block_state_provider_t;

struct block_state_provider__copy_properties_t {
    static constexpr std::string_view type_tag = "copy_properties";
    block_state_t source;
};
struct block_state_provider__dual_noise_t {
    static constexpr std::string_view type_tag = "dual_noise";
    std::int64_t seed;
    noise_t noise;
    float scale;
    noise_t slow_noise;
    float slow_scale;
    int_range_t variety;
    std::vector<block_state_t> states;
};
struct block_state_provider__noise_t {
    static constexpr std::string_view type_tag = "noise";
    std::int64_t seed;
    noise_t noise;
    float scale;
    std::vector<block_state_t> states;
};
struct block_state_provider__noise_threshold_t {
    static constexpr std::string_view type_tag = "noise_threshold";
    std::int64_t seed;
    noise_t noise;
    float scale;
    float threshold;
    float high_chance;
    block_state_t default_state;
    std::vector<block_state_t> low_states;
    std::vector<block_state_t> high_states;
};
struct block_state_provider__random_block_t {
    static constexpr std::string_view type_tag = "random_block";
    std::variant<std::string, block_state_t, std::vector<std::variant<std::string, block_state_t>>> blocks;
};
struct block_state_provider__randomized_int_t {
    static constexpr std::string_view type_tag = "randomized_int";
    std::string property;
    int_provider_t values;
    std::shared_ptr<wrapped_block_state_provider_t> source;
};
struct block_state_provider__rotated_t {
    static constexpr std::string_view type_tag = "rotated";
    std::shared_ptr<wrapped_block_state_provider_t> state;
    block_state_t direction;
};
struct block_state_provider__rule_based_t {
    static constexpr std::string_view type_tag = "rule_based";
    std::shared_ptr<wrapped_block_state_provider_t> fallback;

    struct rule_t {
        block_predicate_t if_true;
        std::shared_ptr<wrapped_block_state_provider_t> then;
    };

    std::vector<rule_t> rules;
};
struct block_state_provider__simple_t {
    static constexpr std::string_view type_tag = "simple";
    block_state_t state;
};
struct block_state_provider__weighted_t {
    static constexpr std::string_view type_tag = "weighted";

    struct entry_t {
        float weight;
        block_state_t data;
    };

    std::vector<entry_t> entries;
};

using typed_block_state_provider_t = std::variant<block_state_provider__copy_properties_t, block_state_provider__dual_noise_t,
    block_state_provider__noise_t, block_state_provider__noise_threshold_t, block_state_provider__random_block_t,
    block_state_provider__randomized_int_t, block_state_provider__rotated_t, block_state_provider__rule_based_t,
    block_state_provider__simple_t, block_state_provider__weighted_t>;
using block_state_provider_t = std::variant<typed_block_state_provider_t, block_state_t>;

struct wrapped_block_state_provider_t {
    block_state_provider_t value;

    struct glaze {
        using R = wrapped_block_state_provider_t;
        static constexpr auto value = &R::value;
    };
};

}
