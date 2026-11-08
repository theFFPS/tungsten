/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <nlohmann/json_fwd.hpp>
#include <tungsten/resource/resource_location.hpp>
#include <tungsten/type/variant.hpp>
#include <optional>
#include <unordered_map>

namespace tungsten {

struct test_environment_t {
    struct function_t {
        std::optional<resource_location_t> m_setup;
        std::optional<resource_location_t> m_teardown;
    };
    struct game_rules_t {
        struct bool_rule_t {
            std::string m_rule;
            bool m_value;

            friend void to_json(nlohmann::json& j, const test_environment_t::game_rules_t::bool_rule_t& val) {
                j = {
                    { "rule", val.m_rule },
                    { "value", val.m_value }
                };
            }
            friend void from_json(const nlohmann::json& j, test_environment_t::game_rules_t::bool_rule_t& val) {
                val.m_rule = j["rule"];
                val.m_value = j["value"];
            }
        };
        struct int_rule_t {
            std::string m_rule;
            int m_value;

            friend void to_json(nlohmann::json& j, const test_environment_t::game_rules_t::int_rule_t& val) {
                j = {
                    { "rule", val.m_rule },
                    { "value", val.m_value }
                };
            }
            friend void from_json(const nlohmann::json& j, test_environment_t::game_rules_t::int_rule_t& val) {
                val.m_rule = j["rule"];
                val.m_value = j["value"];
            }
        };

        std::vector<bool_rule_t> m_bool_rules;
        std::vector<int_rule_t> m_int_rules;
    };
    struct clock_time_t {
        resource_location_t m_clock;
        int m_time;
    };

    resource_location_t m_type;
    std::variant<std::string, function_t, game_rules_t, clock_time_t, std::vector<std::variant<resource_location_t, std::shared_ptr<test_environment_t>>>> m_value;

    friend void to_json(nlohmann::json& j, const test_environment_t& val) {
        std::visit(overloaded{
            [&j, type = val.m_type](const std::string& value) {
                j = {
                    { "type", type },
                    { "weather", value }
                };
            },
            [&j, type = val.m_type](const function_t& value) {
                std::unordered_map<std::string, nlohmann::json> entries = {
                    { "type", type }
                };
                if (value.m_setup.has_value()) entries.insert({ "setup", value.m_setup.value() });
                if (value.m_teardown.has_value()) entries.insert({ "teardown", value.m_teardown.value() });
                j = entries;
            },
            [&j, type = val.m_type](const game_rules_t& value) {
                j = {
                    { "type", type },
                    { "bool_rules", value.m_bool_rules },
                    { "int_rules",  value.m_int_rules  }
                };
            },
            [&j, type = val.m_type](const clock_time_t& value) {
                j = {
                    { "type", type },
                    { "clock", value.m_clock },
                    { "time", value.m_time }
                };
            },
            [&j, type = val.m_type](const std::vector<std::variant<resource_location_t, std::shared_ptr<test_environment_t>>>& value) {
                std::vector<nlohmann::json> definitions;
                for (const auto& entry : value) definitions.push_back(std::visit(overloaded{
                    [](const resource_location_t& value) -> nlohmann::json { return value; },
                    [](const std::shared_ptr<test_environment_t>& value) -> nlohmann::json { return *value.get(); }
                }, entry));
                j = {
                    { "type", type },
                    { "definitions", definitions }
                };
            }
        }, val.m_value);
    }
    friend void from_json(const nlohmann::json& j, test_environment_t& val) {
        val.m_type = j["type"];
        if (val.m_type == resource_location_t("all_of")) {
            std::vector<std::variant<resource_location_t, std::shared_ptr<test_environment_t>>> value;
            for (const auto& entry : j["definitions"]) {
                if (entry.is_string()) value.push_back(resource_location_t(entry));
                else value.push_back(std::make_shared<test_environment_t>((test_environment_t) entry));
            }
            val.m_value = value;
        } else if (val.m_type == resource_location_t("function")) {
            function_t value;
            if (j.contains("setup")) value.m_setup = resource_location_t(j["setup"]);
            if (j.contains("teardown")) value.m_teardown = resource_location_t(j["teardown"]);
            val.m_value = value;
        } else if (val.m_type == resource_location_t("game_rules")) {
            game_rules_t value;
            value.m_bool_rules = std::vector<game_rules_t::bool_rule_t>(j["bool_rules"]);
            value.m_int_rules = std::vector<game_rules_t::int_rule_t>(j["int_rules"]);
            val.m_value = value;
        } else if (val.m_type == resource_location_t("weather")) {
            val.m_value = std::string(j["weather"]);
        } else if (val.m_type == resource_location_t("clock_time")) {
            clock_time_t value;
            value.m_clock = j["clock"];
            value.m_time = j["time"];
            val.m_value = value;
        }
    }
};

struct test_instance_t {
    std::variant<resource_location_t, test_environment_t> m_environment;
    resource_location_t m_structure;
    int m_max_ticks;
    int m_setup_ticks = 0;
    bool m_required = true;
    resource_location_t m_rotation = resource_location_t("none"); // none || 180 || clockwise_90 || counterclockwise_90
    bool m_manual_only = false;
    bool m_sky_access = false;
    int m_max_attempts = 1;
    int m_required_successes = 1;
    resource_location_t m_type; // block_based || function
    std::optional<resource_location_t> m_function;

    friend void to_json(nlohmann::json& j, const test_instance_t& val) {
        std::unordered_map<std::string, nlohmann::json> entries = {
            { "structure", val.m_structure },
            { "setup_ticks", val.m_setup_ticks },
            { "max_ticks", val.m_max_ticks },
            { "required", val.m_required },
            { "rotation", val.m_rotation.m_path },
            { "manual_only", val.m_manual_only },
            { "sky_access", val.m_sky_access },
            { "max_attempts", val.m_max_attempts },
            { "required_successes", val.m_required_successes },
            { "type", val.m_type }
        };
        std::visit(overloaded{
            [&entries](const resource_location_t& value) {
                entries.insert({ "environment", value });
            },
            [&entries](const test_environment_t& value) {
                entries.insert({ "environment", value });
            }
        }, val.m_environment);
        if (val.m_function.has_value()) entries.insert({ "function", val.m_function.value() });
        j = entries;
    }
    friend void from_json(const nlohmann::json& j, test_instance_t& val) {
        if (j.contains("required")) val.m_required = j["required"];
        else val.m_required_successes = true;
        if (j.contains("manual_only")) val.m_manual_only = j["manual_only"];
        else val.m_required_successes = false;
        if (j.contains("sky_access")) val.m_sky_access = j["sky_access"];
        else val.m_required_successes = false;
        if (j.contains("max_attempts")) val.m_max_attempts = j["max_attempts"];
        else val.m_max_attempts = 1;
        if (j.contains("required_successes")) val.m_required_successes = j["required_successes"];
        else val.m_required_successes = 1;
        if (j.contains("rotation")) val.m_rotation = j["rotation"];
        else val.m_rotation = resource_location_t("none");
        if (j.contains("setup_ticks")) val.m_setup_ticks = j["setup_ticks"];
        else val.m_setup_ticks = 0;
        val.m_max_ticks = j["max_ticks"];
        val.m_type = j["type"];
        val.m_structure = j["structure"];
        if (j["environment"].is_string()) val.m_environment = resource_location_t(j["environment"]);
        else val.m_environment = test_environment_t(j["environment"]);
        if (val.m_type == resource_location_t("function")) val.m_function = resource_location_t(j["function"]);
    }
};

}
