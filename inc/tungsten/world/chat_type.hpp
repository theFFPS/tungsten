/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/type/text_component.hpp>
#include <optional>
#include <string>

namespace tungsten {

struct chat_type_t {
    struct chat_decoration_t {
        std::string translation_key;
        std::vector<std::string> parameters; // sender || target || content
        std::optional<text_t> style;
    };

    chat_decoration_t chat;
    chat_decoration_t narration;
};

}
