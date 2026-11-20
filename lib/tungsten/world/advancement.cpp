/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/world/advancement.hpp>

namespace tungsten {

void advancement_t::post_load() {
    if (requirements.empty()) {
        for (const auto& [name, criterion] : criteria) requirements.push_back({ name });
    }
    for (const auto& group : requirements) {
        for (const auto& name : group) {
            if (!criteria.contains(name)) throw jr::error("requirements", "references unknown criterion '" + name + "'");
        }
    }
}

}
