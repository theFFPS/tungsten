/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/resource/resource_location.hpp>
#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

// any JSON object with a mandatory string "type"; the complete object is kept
struct typed_json_t {
    resource_location_t type;
    glz::generic json;

    void from_generic(const glz::generic& j) {
        if (!j.is_object() || !j.contains("type") || !j["type"].is_string()) {
            throw jr::error("", "expected an object with a string 'type'");
        }
        type.from(j["type"].get_string());
        json = j;
    }
    glz::generic to_generic() const { return json; }

    struct glaze {
        using R = typed_json_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

}
