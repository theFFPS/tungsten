/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/resource/resource_location.hpp>

namespace tungsten {

namespace {

bool is_namespace_char(char c) noexcept {
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
}
bool is_path_char(char c) noexcept {
    return is_namespace_char(c) || c == '/';
}

}

void resource_location_t::from(std::string_view combined) {
    std::string_view namespace_s = "minecraft";
    std::string_view path = combined;
    auto colon = combined.find(':');
    if (colon != std::string_view::npos) {
        if (colon != 0) namespace_s = combined.substr(0, colon);
        path = combined.substr(colon + 1);
    }
    if (!is_valid_namespace(namespace_s) || !is_valid_path(path)) {
        throw std::runtime_error("resource_location_t::from error: invalid resource location '" + std::string(combined) + "' format. valid: 'namespace:path/object'");
    }
    m_namespace = namespace_s;
    m_path = path;
}
std::string resource_location_t::to_string() const noexcept {
    return m_namespace + ":" + m_path;
}

bool resource_location_t::is_valid_namespace(std::string_view value) noexcept {
    for (char c : value) if (!is_namespace_char(c)) return false;
    return true;
}
bool resource_location_t::is_valid_path(std::string_view value) noexcept {
    for (char c : value) if (!is_path_char(c)) return false;
    return true;
}
bool resource_location_t::is_valid(std::string_view combined) noexcept {
    auto colon = combined.find(':');
    if (colon == std::string_view::npos) return is_valid_path(combined);
    return is_valid_namespace(combined.substr(0, colon)) && is_valid_path(combined.substr(colon + 1));
}

bool resource_location_t::operator==(const resource_location_t& other) const noexcept {
    return other.m_namespace == m_namespace && other.m_path == m_path;
}
bool resource_location_t::operator!=(const resource_location_t& other) const noexcept {
    return !(*this == other);
}
bool resource_location_t::operator>(const resource_location_t& other) const noexcept {
    return std::tie(m_namespace, m_path) > std::tie(other.m_namespace, other.m_path);
}
bool resource_location_t::operator<(const resource_location_t& other) const noexcept {
    return std::tie(m_namespace, m_path) < std::tie(other.m_namespace, other.m_path);
}

}
