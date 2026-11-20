/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <variant>
#include <tungsten/util/json_reflect.hpp>
#include <tungsten/type/variant.hpp>
#include <tungsten/resource/resource_location.hpp>
#include "predicate.hpp"
#include "level_based_value.hpp"

namespace tungsten {

struct wrapped_context_number_provider_t;

struct context_number_provider__abs_t {
    static constexpr std::string_view type_tag = "abs";
    std::shared_ptr<wrapped_context_number_provider_t> input;
};
struct context_number_provider__avg_t {
    static constexpr std::string_view type_tag = "avg";
    std::vector<wrapped_context_number_provider_t> inputs;
};
struct context_number_provider__binomial_t {
    static constexpr std::string_view type_tag = "binomial";
    std::shared_ptr<wrapped_context_number_provider_t> n;
    std::shared_ptr<wrapped_context_number_provider_t> p;
};
struct context_number_provider__ceil_t {
    static constexpr std::string_view type_tag = "ceil";
    std::shared_ptr<wrapped_context_number_provider_t> input;
};
struct context_number_provider__conditional_t {
    static constexpr std::string_view type_tag = "conditional";
    std::variant<resource_location_t, predicate_t> condition;
    std::shared_ptr<wrapped_context_number_provider_t> on_true;
    std::optional<std::shared_ptr<wrapped_context_number_provider_t>> on_false;
};
struct context_number_provider__constant_t {
    static constexpr std::string_view type_tag = "constant";
    std::variant<int, float> value;
};
struct context_number_provider__cos_t {
    static constexpr std::string_view type_tag = "cos";
    std::shared_ptr<wrapped_context_number_provider_t> input;
};
struct context_number_provider__sub_t {
    static constexpr std::string_view type_tag = "sub";
    std::shared_ptr<wrapped_context_number_provider_t> left;
    std::shared_ptr<wrapped_context_number_provider_t> right;
};
struct context_number_provider__enchantment_level_t {
    static constexpr std::string_view type_tag = "enchantment_level";
    level_based_value_t amount;
};
struct context_number_provider__floor_t {
    static constexpr std::string_view type_tag = "floor";
    std::shared_ptr<wrapped_context_number_provider_t> input;
};
struct context_number_provider__from_float_t {
    static constexpr std::string_view type_tag = "from_float";
    std::shared_ptr<wrapped_context_number_provider_t> input;
};
struct context_number_provider__from_int_t {
    static constexpr std::string_view type_tag = "from_int";
    std::shared_ptr<wrapped_context_number_provider_t> input;
};
struct context_number_provider__length_t {
    static constexpr std::string_view type_tag = "length";
    std::vector<wrapped_context_number_provider_t> inputs;
};
struct context_number_provider__max_t {
    static constexpr std::string_view type_tag = "max";
    std::vector<wrapped_context_number_provider_t> inputs;
};
struct context_number_provider__min_t {
    static constexpr std::string_view type_tag = "min";
    std::vector<wrapped_context_number_provider_t> inputs;
};
struct context_number_provider__mod_t {
    static constexpr std::string_view type_tag = "mod";
    std::shared_ptr<wrapped_context_number_provider_t> left;
    std::shared_ptr<wrapped_context_number_provider_t> right;
};
struct context_number_provider__floor_mod_t {
    static constexpr std::string_view type_tag = "floor_mod";
    std::shared_ptr<wrapped_context_number_provider_t> left;
    std::shared_ptr<wrapped_context_number_provider_t> right;
};
struct context_number_provider__environment_attribute_t {
    static constexpr std::string_view type_tag = "environment_attribute";
    resource_location_t attribute;
};
struct context_number_provider__negate_t {
    static constexpr std::string_view type_tag = "negate";
    std::shared_ptr<wrapped_context_number_provider_t> input;
};
struct context_number_provider__number_dispatcher_t {
    struct case_t {
        std::variant<resource_location_t, predicate_t> condition;
        std::shared_ptr<wrapped_context_number_provider_t> value;
    };

    static constexpr std::string_view type_tag = "number_dispatcher";
    std::vector<case_t> cases;
    std::optional<std::shared_ptr<wrapped_context_number_provider_t>> default_;
};
struct context_number_provider__pow_t {
    static constexpr std::string_view type_tag = "pow";
    std::shared_ptr<wrapped_context_number_provider_t> base;
    std::shared_ptr<wrapped_context_number_provider_t> exponent;
};
struct context_number_provider__mul_t {
    static constexpr std::string_view type_tag = "mul";
    std::vector<wrapped_context_number_provider_t> inputs;
};
struct context_number_provider__div_t {
    static constexpr std::string_view type_tag = "div";
    std::shared_ptr<wrapped_context_number_provider_t> left;
    std::shared_ptr<wrapped_context_number_provider_t> right;
};
struct context_number_provider__round_t {
    static constexpr std::string_view type_tag = "round";
    std::shared_ptr<wrapped_context_number_provider_t> input;
};
struct context_number_provider__sin_t {
    static constexpr std::string_view type_tag = "sin";
    std::shared_ptr<wrapped_context_number_provider_t> input;
};
struct context_number_provider__sqrt_t {
    static constexpr std::string_view type_tag = "sqrt";
    std::shared_ptr<wrapped_context_number_provider_t> input;
};
struct context_number_provider__score_t {
    struct target_t {
        resource_location_t type;
        std::optional<std::string> name;
        std::optional<std::string> target; // this || attacker || direct_attacker || attacking_player || target_entity || interacting_entity
    };

    static constexpr std::string_view type_tag = "score";
    std::optional<std::variant<std::string, target_t>> target;
    std::optional<std::string> score;
    std::optional<std::shared_ptr<wrapped_context_number_provider_t>> fallback;
};
struct context_number_provider__storage_t {
    static constexpr std::string_view type_tag = "storage";
    std::optional<resource_location_t> storage;
    std::optional<std::string> path;
    std::optional<std::shared_ptr<wrapped_context_number_provider_t>> fallback;
};
struct context_number_provider__add_t {
    static constexpr std::string_view type_tag = "add";
    std::vector<wrapped_context_number_provider_t> inputs;
};
struct context_number_provider__truncate_t {
    static constexpr std::string_view type_tag = "truncate";
    std::shared_ptr<wrapped_context_number_provider_t> input;
};
struct context_number_provider__uniform_t {
    static constexpr std::string_view type_tag = "uniform";
    std::optional<std::shared_ptr<wrapped_context_number_provider_t>> min;
    std::optional<std::shared_ptr<wrapped_context_number_provider_t>> max;
};
struct context_number_provider__weighted_list_t {
    struct entry_t {
        std::shared_ptr<wrapped_context_number_provider_t> data;
        float weight;
    };

    static constexpr std::string_view type_tag = "weighted_list";
    std::vector<entry_t> distribution;
};
using complex_context_number_provider_t = std::variant<context_number_provider__abs_t, context_number_provider__avg_t, context_number_provider__binomial_t,
    context_number_provider__ceil_t, context_number_provider__conditional_t, context_number_provider__constant_t, context_number_provider__cos_t,
    context_number_provider__sub_t, context_number_provider__enchantment_level_t, context_number_provider__floor_t, context_number_provider__from_float_t,
    context_number_provider__from_int_t, context_number_provider__length_t, context_number_provider__max_t, context_number_provider__min_t,
    context_number_provider__mod_t, context_number_provider__floor_mod_t, context_number_provider__environment_attribute_t, context_number_provider__negate_t,
    context_number_provider__number_dispatcher_t, context_number_provider__pow_t, context_number_provider__mul_t, context_number_provider__div_t,
    context_number_provider__round_t, context_number_provider__sin_t, context_number_provider__sqrt_t, context_number_provider__score_t,
    context_number_provider__storage_t, context_number_provider__add_t, context_number_provider__truncate_t, context_number_provider__uniform_t,
    context_number_provider__weighted_list_t>;
using context_number_provider_t = std::variant<complex_context_number_provider_t, resource_location_t, int, float>;

struct wrapped_context_number_provider_t {
    context_number_provider_t value;

    struct glaze {
        using R = wrapped_context_number_provider_t;
        static constexpr auto value = &R::value;
    };
};

}

// members named like a C++ keyword carry a trailing underscore which is not part of the key
template <> struct glz::meta<tungsten::context_number_provider__number_dispatcher_t> {
    static constexpr std::string_view rename_key(const std::string_view key) { return key.ends_with('_') ? key.substr(0, key.size() - 1) : key; }
};
