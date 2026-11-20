/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <optional>
#include <tungsten/resource/resource_location.hpp>
#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

// "minecraft:entity.cat.ambient" or { "sound_id": "...", "range": 16.0 }
struct sound_event_t {
    resource_location_t sound_id;
    std::optional<float> range;

    void from_generic(const glz::generic& j) {
        if (j.is_string()) {
            sound_id.from(j.get_string());
            range.reset();
        } else if (j.is_object()) {
            if (!j.contains("sound_id")) throw jr::error("", "missing required key 'sound_id'");
            sound_id.from(j["sound_id"].get_string());
            if (j.contains("range")) range = static_cast<float>(j["range"].get_number());
            else range.reset();
        } else {
            throw jr::error("", "expected a sound event (string or object)");
        }
    }
    glz::generic to_generic() const {
        if (!range.has_value()) return glz::generic(sound_id.to_string());
        glz::generic j = glz::generic::object_t{};
        j["sound_id"] = sound_id.to_string();
        j["range"] = static_cast<double>(*range);
        return j;
    }

    struct glaze {
        using R = sound_event_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

}
