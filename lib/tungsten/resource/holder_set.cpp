/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/resource/holder_set.hpp>

namespace tungsten {

std::string resource_or_tag_t::to_string() const {
    return is_tag ? "#" + id.to_string() : id.to_string();
}
resource_or_tag_t resource_or_tag_t::parse(std::string_view text) {
    if (text.starts_with('#')) return resource_or_tag_t(true, resource_location_t(text.substr(1)));
    return resource_or_tag_t(false, resource_location_t(text));
}

}
