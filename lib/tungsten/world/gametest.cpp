/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/world/gametest.hpp>

namespace tungsten {

namespace {

[[noreturn]] void fail(const std::string& message) {
    throw jr::error("", message);
}
template<class T> T required(const nlohmann::json& j, const char* key) {
    auto it = j.find(key);
    if (it == j.end()) fail(std::string("missing required key '") + key + "'");
    try {
        return it->get<T>();
    } catch (const jr::error& e) {
        throw jr::error(std::string(key) + (e.m_path.empty() ? "" : "." + e.m_path), e.m_message);
    }
}
template<class T> void optional_into(const nlohmann::json& j, const char* key, T& out) {
    auto it = j.find(key);
    if (it == j.end() || it->is_null()) return;
    try {
        out = it->get<T>();
    } catch (const jr::error& e) {
        throw jr::error(std::string(key) + (e.m_path.empty() ? "" : "." + e.m_path), e.m_message);
    }
}

}

void to_json(nlohmann::json& j, const test_environment_t::definition_t& val) {
    if (val.m_reference.has_value()) j = *val.m_reference;
    else if (val.m_inline) j = *val.m_inline;
    else j = nullptr;
}
void from_json(const nlohmann::json& j, test_environment_t::definition_t& val) {
    val = test_environment_t::definition_t();
    if (j.is_string()) val.m_reference = j.get<resource_location_t>();
    else val.m_inline = std::make_shared<test_environment_t>(j.get<test_environment_t>());
}

void to_json(nlohmann::json& j, const test_environment_t& val) {
    std::visit([&j](const auto& value) {
        using value_t = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<value_t, test_environment_t::all_of_t>) j = { { "definitions", value.m_definitions } };
        else j = value;
    }, val.m_value);
    j["type"] = val.m_type;
}
void from_json(const nlohmann::json& j, test_environment_t& val) {
    if (!j.is_object()) fail(std::string("expected an object, got ") + j.type_name());
    val.m_type = required<resource_location_t>(j, "type");
    const std::string& type = val.m_type.m_path;
    if (val.m_type.m_namespace != "minecraft") fail("unknown test environment type '" + val.m_type.to_string() + "'");
    if (type == "all_of") {
        val.m_value = test_environment_t::all_of_t { required<std::vector<test_environment_t::definition_t>>(j, "definitions") };
    } else if (type == "function") val.m_value = j.get<test_environment_t::function_t>();
    else if (type == "game_rules") val.m_value = j.get<test_environment_t::game_rules_t>();
    else if (type == "weather") val.m_value = j.get<test_environment_t::weather_t>();
    else if (type == "clock_time") val.m_value = j.get<test_environment_t::clock_time_t>();
    else fail("unknown test environment type '" + val.m_type.to_string() + "'");
}

void to_json(nlohmann::json& j, const test_instance_t& val) {
    j = {
        { "structure", val.m_structure },
        { "max_ticks", val.m_max_ticks },
        { "setup_ticks", val.m_setup_ticks },
        { "required", val.m_required },
        { "rotation", val.m_rotation },
        { "manual_only", val.m_manual_only },
        { "sky_access", val.m_sky_access },
        { "max_attempts", val.m_max_attempts },
        { "required_successes", val.m_required_successes },
        { "type", val.m_type }
    };
    std::visit([&j](const auto& value) { j["environment"] = value; }, val.m_environment);
    if (val.m_function.has_value()) j["function"] = *val.m_function;
}
void from_json(const nlohmann::json& j, test_instance_t& val) {
    if (!j.is_object()) fail(std::string("expected an object, got ") + j.type_name());
    val = test_instance_t();
    val.m_type = required<resource_location_t>(j, "type");
    val.m_structure = required<resource_location_t>(j, "structure");
    val.m_max_ticks = required<int>(j, "max_ticks");
    auto environment = j.find("environment");
    if (environment == j.end()) fail("missing required key 'environment'");
    if (environment->is_string()) val.m_environment = environment->get<resource_location_t>();
    else val.m_environment = environment->get<test_environment_t>();
    optional_into(j, "setup_ticks", val.m_setup_ticks);
    optional_into(j, "required", val.m_required);
    optional_into(j, "rotation", val.m_rotation);
    optional_into(j, "manual_only", val.m_manual_only);
    optional_into(j, "sky_access", val.m_sky_access);
    optional_into(j, "max_attempts", val.m_max_attempts);
    optional_into(j, "required_successes", val.m_required_successes);
    if (val.m_type.m_path == "function") val.m_function = required<resource_location_t>(j, "function");
    else optional_into(j, "function", val.m_function);
}

}
