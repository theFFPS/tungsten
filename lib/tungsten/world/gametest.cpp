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

using json = glz::generic;

[[noreturn]] void fail(const std::string& message) {
    throw jr::error("", message);
}
template<class T> T req(const json& j, const char* key) {
    if (!j.is_object() || !j.contains(key)) fail(std::string("missing required key '") + key + "'");
    try {
        return jr::from_generic<T>(j[key]);
    } catch (const jr::error& e) {
        throw jr::error(std::string(key) + (e.m_path.empty() ? "" : "." + e.m_path), e.m_message);
    }
}
template<class T> void opt_into(const json& j, const char* key, T& out) {
    if (!j.contains(key) || j[key].is_null()) return;
    try {
        out = jr::from_generic<T>(j[key]);
    } catch (const jr::error& e) {
        throw jr::error(std::string(key) + (e.m_path.empty() ? "" : "." + e.m_path), e.m_message);
    }
}

}

void test_environment_t::definition_t::from_generic(const json& j) {
    *this = definition_t();
    if (j.is_string()) reference = resource_location_t(j.get_string());
    else inline_ = std::make_shared<test_environment_t>(jr::from_generic<test_environment_t>(j));
}
json test_environment_t::definition_t::to_generic() const {
    if (reference.has_value()) return json(reference->to_string());
    if (inline_) return jr::to_generic(*inline_);
    return json(nullptr);
}

void test_environment_t::from_generic(const json& j) {
    if (!j.is_object()) fail("expected an object");
    type = req<resource_location_t>(j, "type");
    const std::string& name = type.m_path;
    if (type.m_namespace != "minecraft") fail("unknown test environment type '" + type.to_string() + "'");
    if (name == "all_of") {
        value = all_of_t { req<std::vector<definition_t>>(j, "definitions") };
    } else if (name == "function") value = jr::from_generic<function_t>(j);
    else if (name == "game_rules") value = jr::from_generic<game_rules_t>(j);
    else if (name == "weather") value = jr::from_generic<weather_t>(j);
    else if (name == "clock_time") value = jr::from_generic<clock_time_t>(j);
    else fail("unknown test environment type '" + type.to_string() + "'");
}
json test_environment_t::to_generic() const {
    json j = std::visit([](const auto& v) -> json {
        using value_t = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<value_t, all_of_t>) {
            json o = json::object_t{};
            o["definitions"] = jr::to_generic(v.definitions);
            return o;
        } else return jr::to_generic(v);
    }, value);
    j["type"] = type.to_string();
    return j;
}

void test_instance_t::from_generic(const json& j) {
    if (!j.is_object()) fail("expected an object");
    *this = test_instance_t();
    type = req<resource_location_t>(j, "type");
    structure = req<resource_location_t>(j, "structure");
    max_ticks = req<int>(j, "max_ticks");
    if (!j.contains("environment")) fail("missing required key 'environment'");
    if (j["environment"].is_string()) environment = resource_location_t(j["environment"].get_string());
    else environment = req<test_environment_t>(j, "environment");
    opt_into(j, "setup_ticks", setup_ticks);
    opt_into(j, "required", required);
    opt_into(j, "rotation", rotation);
    opt_into(j, "manual_only", manual_only);
    opt_into(j, "sky_access", sky_access);
    opt_into(j, "max_attempts", max_attempts);
    opt_into(j, "required_successes", required_successes);
    if (type.m_path == "function") function = req<resource_location_t>(j, "function");
    else opt_into(j, "function", function);
}
json test_instance_t::to_generic() const {
    json j = json::object_t{};
    j["structure"] = structure.to_string();
    j["max_ticks"] = static_cast<double>(max_ticks);
    j["setup_ticks"] = static_cast<double>(setup_ticks);
    j["required"] = required;
    j["rotation"] = rotation;
    j["manual_only"] = manual_only;
    j["sky_access"] = sky_access;
    j["max_attempts"] = static_cast<double>(max_attempts);
    j["required_successes"] = static_cast<double>(required_successes);
    j["type"] = type.to_string();
    std::visit([&j](const auto& v) { j["environment"] = jr::to_generic(v); }, environment);
    if (function.has_value()) j["function"] = function->to_string();
    return j;
}

}
