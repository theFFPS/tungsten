/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/resource/resource_location.hpp>

namespace tungsten {

struct sound_event_t {
    resource_location_t m_sound_id;
    std::optional<float> m_range;

    friend void to_json(nlohmann::json& j, const sound_event_t& val) {
        if (val.m_range.has_value()) j = {
            { "sound_id", val.m_sound_id },
            { "range", val.m_range.value() }
        }; else j = val.m_sound_id;
    }
    friend void from_json(const nlohmann::json& j, sound_event_t& val) {
        if (j.is_string()) val.m_sound_id = j;
        else {
            val.m_sound_id = j.at("sound_id");
            if (j.contains("range")) val.m_range = j["range"];
        }
    }
};

}
