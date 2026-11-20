/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/type/text_component.hpp>
#include <tungsten/type/sound_event.hpp>
#include <tungsten/resource/resource_location.hpp>
#include <optional>

namespace tungsten {

struct instrument_t {
    sound_event_t sound_event;
    std::optional<float> use_duration;
    float range;
    text_t description;
};
struct jukebox_song_t {
    sound_event_t sound_event;
    text_t description;
    float length_in_seconds;
    int comparator_output;
};

}
