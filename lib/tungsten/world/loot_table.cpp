/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/world/loot_table.hpp>

namespace tungsten {

void item_modifier_t::from_generic(const glz::generic& j) {
    functions.clear();
    reference.reset();
    if (j.is_string()) reference = resource_location_t(j.get_string());
    else if (j.is_object()) functions.push_back(j);
    else if (j.is_array()) {
        for (const auto& f : j.get_array()) {
            if (!f.is_object()) throw jr::error("", "item modifier functions must be objects");
            functions.push_back(f);
        }
    } else throw jr::error("", "expected an object or an array");
}
glz::generic item_modifier_t::to_generic() const {
    if (reference) return glz::generic(reference->to_string());
    if (functions.size() == 1) return functions.front();
    return glz::generic(glz::generic::array_t(functions.begin(), functions.end()));
}

}
