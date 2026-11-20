/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <compare>
#include <string>
#include <string_view>
#include <stdexcept>
#include <glaze/glaze.hpp>

namespace tungsten {

struct resource_location_t {
    std::string m_namespace;
    std::string m_path;

    resource_location_t() : m_namespace("minecraft"), m_path("placeholder") {}
    resource_location_t(std::string_view namespace_s, std::string_view path) : m_namespace(namespace_s), m_path(path) {}
    resource_location_t(std::string_view combined) {
        from(combined);
    }

    // parses 'namespace:path' (namespace defaults to 'minecraft'); throws std::runtime_error when the location is malformed
    void from(std::string_view combined);
    std::string to_string() const noexcept;

    static bool is_valid_namespace(std::string_view value) noexcept;
    static bool is_valid_path(std::string_view value) noexcept;
    static bool is_valid(std::string_view combined) noexcept;

    bool operator==(const resource_location_t& other) const noexcept;
    bool operator!=(const resource_location_t& other) const noexcept;
    bool operator>(const resource_location_t& other) const noexcept;
    bool operator<(const resource_location_t& other) const noexcept;

    // glaze: serialized as a single "namespace:path" string
    void glaze_from(const std::string& s) { from(s); }
    std::string glaze_to() const { return to_string(); }
};
struct resource_key_t {
    resource_location_t m_registry;
    resource_location_t m_object;
};

}

// glz::meta for resource_location_t: a plain JSON string in both directions
template <>
struct glz::meta<tungsten::resource_location_t> {
    using T = tungsten::resource_location_t;
    using mimic = std::string; // behaves as a JSON string: usable as an object key and in variant deduction
    static constexpr auto value = glz::custom<&T::glaze_from, &T::glaze_to>;
};

namespace std {
    template<> struct hash<tungsten::resource_location_t> {
        std::size_t operator()(const tungsten::resource_location_t& k) const {
            std::size_t h1 = std::hash<std::string>{}(k.m_namespace);
            std::size_t h2 = std::hash<std::string>{}(k.m_path);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
}
