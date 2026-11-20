/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/world/function.hpp>

#include <stdexcept>

namespace tungsten {

namespace {

bool valid_utf8(std::string_view s) {
    std::size_t i = 0;
    while (i < s.size()) {
        auto c = static_cast<std::uint8_t>(s[i]);
        std::size_t extra;
        if (c < 0x80) extra = 0;
        else if ((c >> 5) == 0x6) extra = 1;
        else if ((c >> 4) == 0xE) extra = 2;
        else if ((c >> 3) == 0x1E) extra = 3;
        else return false;
        for (std::size_t k = 1; k <= extra; ++k) {
            if (i + k >= s.size() || (static_cast<std::uint8_t>(s[i + k]) & 0xC0) != 0x80) return false;
        }
        i += extra + 1;
    }
    return true;
}

}

function_t function_t::parse(std::span<const std::uint8_t> bytes) {
    std::string_view text (reinterpret_cast<const char*>(bytes.data()), bytes.size());
    if (text.starts_with("\xEF\xBB\xBF")) text.remove_prefix(3);
    if (!valid_utf8(text)) throw std::runtime_error("function_t::parse error: the function is not valid UTF-8");

    function_t result;
    std::size_t line_number = 0;
    std::size_t pos = 0;
    std::string pending;
    std::size_t pending_line = 0;
    bool continuing = false;
    while (pos < text.size()) {
        std::size_t end = text.find('\n', pos);
        std::string_view line = text.substr(pos, end == std::string_view::npos ? std::string_view::npos : end - pos);
        pos = end == std::string_view::npos ? text.size() : end + 1;
        ++line_number;
        if (line.ends_with('\r')) line.remove_suffix(1);

        if (!continuing) {
            std::size_t first = line.find_first_not_of(" \t");
            if (first == std::string_view::npos) continue;
            line.remove_prefix(first);
            if (line.starts_with('#')) continue;
            pending_line = line_number;
        } else {
            std::size_t first = line.find_first_not_of(" \t");
            line = first == std::string_view::npos ? std::string_view() : line.substr(first);
        }
        bool continued = line.ends_with('\\');
        if (continued) line.remove_suffix(1);
        pending += line;
        continuing = continued;
        if (continued) continue;

        std::string command = std::move(pending);
        pending.clear();
        bool macro = command.starts_with('$');
        if (!macro && command.starts_with('/')) command.erase(0, 1);
        result.m_commands.push_back({ pending_line, std::move(command), macro });
    }
    if (continuing) throw std::runtime_error("function_t::parse error: line " + std::to_string(pending_line) + ": dangling line continuation");
    return result;
}

}
