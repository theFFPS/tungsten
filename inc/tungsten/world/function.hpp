/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace tungsten {

struct function_t {
    struct line_t {
        std::size_t m_line_number;
        std::string m_text;
        bool m_macro = false;
    };

    std::vector<line_t> m_commands;
    static function_t parse(std::span<const std::uint8_t> bytes);
};

}
