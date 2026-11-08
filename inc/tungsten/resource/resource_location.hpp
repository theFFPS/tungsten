/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <string>
#include <string_view>
#include <nlohmann/json.hpp>

namespace tungsten {

struct resource_location_t {
    std::string m_namespace;
    std::string m_path;

    resource_location_t() : m_namespace("minecraft"), m_path("placeholder") {}
    resource_location_t(std::string_view namespace_s, std::string_view path) : m_namespace(namespace_s), m_path(path) {}
    resource_location_t(std::string_view combined) {
        from(combined);
    }

    void from(std::string_view combined);
    std::string to_string() const noexcept;

    bool operator==(const resource_location_t& other) const noexcept;
    bool operator!=(const resource_location_t& other) const noexcept;
    bool operator>(const resource_location_t& other) const noexcept;
    bool operator<(const resource_location_t& other) const noexcept;

    friend void to_json(nlohmann::json& j, const resource_location_t& rl) {
        j = rl.to_string();
    }
    friend void from_json(const nlohmann::json& j, resource_location_t& rl) {
        rl.from(std::string(j));
    }
};

}

namespace std {
    template<> struct hash<tungsten::resource_location_t> {
        std::size_t operator()(const tungsten::resource_location_t& k) const {
            std::size_t h1 = std::hash<std::string>{}(k.m_namespace);
            std::size_t h2 = std::hash<std::string>{}(k.m_path);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
}
