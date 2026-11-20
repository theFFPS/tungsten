/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <vector>
#include <tungsten/type/sound_event.hpp>
#include "block_state_provider.hpp"

namespace tungsten {

struct block_transformer_rule_t {
    block_state_provider_t block_state_provider;
    std::optional<sound_event_t> sound;
    std::string particle = "none"; // none || scrape || wax_on || wax_off
    std::vector<std::string> disallowed_faces = {}; // up || down || north || east || south || west
    std::optional<resource_location_t> loot;
    std::string drop_strategy = "from_middle"; // from_middle || clicked_face
    bool update_from_neighbors = true;
    std::optional<std::string> transform_type; // single_block || copper_chest
    bool consume_on_use = true;
    int item_damage_per_use = 1;
};
using block_transformer_t = std::vector<block_transformer_rule_t>;

}
