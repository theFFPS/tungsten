/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/resource/resource_location.hpp>
#include <stdexcept>

namespace tungsten {

void resource_location_t::from(std::string_view combined) {
    m_namespace = "minecraft";
    auto sections = combined | std::views::split(':') | std::ranges::to<std::vector<std::string>>();
    if (sections.size() > 2) {
        throw std::runtime_error("resource_location_t::from error: invalid resource location '" + std::string(combined) + "' format. valid: 'namespace:path/object'");
    }
    if (sections.size() == 2) m_namespace = sections.front();
    m_path = sections.back();
}
std::string resource_location_t::to_string() const noexcept {
    return m_namespace + ":" + m_path;
}
bool resource_location_t::operator==(const resource_location_t& other) const noexcept {
    return other.m_namespace == m_namespace && other.m_path == m_path;
}
bool resource_location_t::operator!=(const resource_location_t& other) const noexcept {
    return other.m_namespace != m_namespace || other.m_path != m_path;
}
bool resource_location_t::operator>(const resource_location_t& other) const noexcept {
    return other.to_string() > to_string();
}
bool resource_location_t::operator<(const resource_location_t& other) const noexcept {
    return other.to_string() < to_string();
}

}
