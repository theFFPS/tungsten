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

struct world_clock_t {};
struct timeline_t {
    struct time_marker_t {
        std::optional<int> ticks;
        bool show_in_commands = false;

        // an integer (ticks) or { "ticks": .., "show_in_commands": .. }
        void from_generic(const glz::generic& j) {
            *this = time_marker_t();
            if (j.is_number()) ticks = static_cast<int>(j.get_number());
            else if (j.is_object()) {
                if (j.contains("ticks")) ticks = static_cast<int>(j["ticks"].get_number());
                if (j.contains("show_in_commands")) show_in_commands = j["show_in_commands"].get_boolean();
            } else throw jr::error("", "expected an integer or an object");
        }
        glz::generic to_generic() const {
            if (ticks.has_value() && !show_in_commands) return glz::generic(static_cast<double>(*ticks));
            glz::generic j = glz::generic::object_t{};
            if (ticks.has_value()) j["ticks"] = static_cast<double>(*ticks);
            if (show_in_commands) j["show_in_commands"] = true;
            return j;
        }

        struct glaze {
            using R = time_marker_t;
            static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
        };
    };

    resource_location_t clock;
    std::optional<int> period_ticks;
    std::map<resource_location_t, time_marker_t> time_markers = {};
    std::map<resource_location_t, glz::generic> tracks = {}; // attribute -> { keyframes, modifier, ease } //TODO! implement environment attributes
};

}
