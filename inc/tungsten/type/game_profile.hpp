/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <optional>
#include <array>
#include <string>
#include <vector>

namespace tungsten {

struct game_profile_property_t {
    std::string name;
    std::string value;
    std::optional<std::string> signature;
};
struct game_profile_t {
    std::optional<std::string> name;
    std::optional<std::array<int, 4>> id;
    std::optional<std::vector<game_profile_property_t>> properties;
    std::string texture = "minecraft:entity/player/slim/alex";
    std::optional<std::string> cape;
    std::string elytra = "minecraft:entity/equipment/wings/elytra";
    std::string model = "slim";
};

}
