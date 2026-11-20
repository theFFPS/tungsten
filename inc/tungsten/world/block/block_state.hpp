/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <tungsten/resource/resource_location.hpp>

namespace tungsten {

struct block_state_t {
    std::optional<resource_location_t> name;
    std::optional<std::unordered_map<std::string, std::string>> Properties;
    std::optional<resource_location_t> id;
    std::optional<std::unordered_map<std::string, std::string>> properties;
};

}
