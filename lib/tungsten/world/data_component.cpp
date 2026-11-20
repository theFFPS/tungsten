/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/world/data_component.hpp>
#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

void data_components_t::from_generic(const glz::generic& j) {
    *this = data_components_t();
    if (!j.is_object()) throw jr::error("", "expected an object");
    for (const auto& [key, val] : j.get_object()) {
        try {
            if (key.starts_with('!')) removed.emplace_back(std::string_view(key).substr(1));
            else value.emplace(resource_location_t(key), val);
        } catch (const std::exception& e) {
            throw jr::error("[" + key + "]", e.what());
        }
    }
}
glz::generic data_components_t::to_generic() const {
    glz::generic j = glz::generic::object_t{};
    for (const auto& [id, val] : value) j[id.to_string()] = val;
    for (const auto& id : removed) j["!" + id.to_string()] = glz::generic::object_t{};
    return j;
}

}
