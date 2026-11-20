/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/util/json_reflect.hpp>
#include <tungsten/resource/resource_location.hpp>

namespace tungsten {

// a resource which is kept as validated JSON of any shape (object, array, number...)
struct raw_resource_t {
    glz::generic json;

    void from_generic(const glz::generic& j) { json = j; }
    glz::generic to_generic() const { return json; }

    struct glaze {
        using R = raw_resource_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

// an object resource with a mandatory "type" discriminator, the rest of the object is kept as JSON
struct typed_resource_t {
    resource_location_t type;
    glz::generic json; // the complete object (the "type" key included)

    void from_generic(const glz::generic& j) {
        if (!j.is_object()) throw jr::error("", "expected an object");
        if (!j.contains("type") || !j["type"].is_string()) throw jr::error("", "missing required key 'type'");
        type.from(j["type"].get_string());
        json = j;
    }
    glz::generic to_generic() const { return json; }

    struct glaze {
        using R = typed_resource_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

}
