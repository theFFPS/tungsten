/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <tungsten/resource/resource_location.hpp>
#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

struct test_environment_t {
    struct function_t {
        std::optional<resource_location_t> setup;
        std::optional<resource_location_t> teardown;
    };
    struct game_rules_t {
        struct bool_rule_t {
            std::string rule;
            bool value;
        };
        struct int_rule_t {
            std::string rule;
            int value;
        };

        std::vector<bool_rule_t> bool_rules = {};
        std::vector<int_rule_t> int_rules = {};
    };
    struct weather_t {
        std::string weather; // clear || rain || thunder
    };
    struct clock_time_t {
        resource_location_t clock;
        int time;
    };
    struct definition_t {
        std::optional<resource_location_t> reference;
        std::shared_ptr<test_environment_t> inline_;

        // a reference ("id") or an inline environment (object)
        void from_generic(const glz::generic& j);
        glz::generic to_generic() const;

        struct glaze {
            using R = definition_t;
            static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
        };
    };
    struct all_of_t {
        std::vector<definition_t> definitions;
    };

    resource_location_t type; // all_of || function || game_rules || weather || clock_time
    std::variant<all_of_t, function_t, game_rules_t, weather_t, clock_time_t> value;

    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = test_environment_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};
struct test_instance_t {
    std::variant<resource_location_t, test_environment_t> environment;
    resource_location_t structure;
    int max_ticks;
    int setup_ticks = 0;
    bool required = true;
    std::string rotation = "none"; // none || clockwise_90 || 180 || counterclockwise_90
    bool manual_only = false;
    bool sky_access = false;
    int max_attempts = 1;
    int required_successes = 1;
    resource_location_t type; // block_based || function
    std::optional<resource_location_t> function; // function tests only

    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = test_instance_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

}
