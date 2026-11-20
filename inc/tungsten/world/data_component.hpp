/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <map>
#include <vector>
#include <tungsten/util/json_reflect.hpp>
#include <tungsten/resource/resource_location.hpp>

namespace tungsten {

// TODO! implement data components
struct data_components_t {
    std::map<resource_location_t, glz::generic> value;
    std::vector<resource_location_t> removed;

    bool empty() const noexcept { return value.empty() && removed.empty(); }
    bool contains(const resource_location_t& id) const { return value.contains(id); }
    const glz::generic* find(const resource_location_t& id) const {
        auto it = value.find(id);
        return it == value.end() ? nullptr : &it->second;
    }

    // { "<id>": <value>, "!<id>": {} } where the "!" form removes a component
    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = data_components_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

}
