/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/world/entity/entity.hpp>
#include <tungsten/world/block/block_entity.hpp>

namespace tungsten {

void wrapped_wrapped_block_entity_t::from_generic(const glz::generic& j) {
    value = std::make_shared<wrapped_block_entity_t>(jr::from_generic<wrapped_block_entity_t>(j));
}
glz::generic wrapped_wrapped_block_entity_t::to_generic() const {
    return jr::to_generic(*value);
}

}
