/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <string>
#include <vector>
#include <tungsten/util/json_reflect.hpp>
#include "resource_location.hpp"

namespace tungsten {

// "#namespace:path" (a tag reference) or "namespace:path" (a plain id)
struct resource_or_tag_t {
    bool is_tag = false;
    resource_location_t id;

    resource_or_tag_t() = default;
    resource_or_tag_t(bool is_tag, resource_location_t id) : is_tag(is_tag), id(std::move(id)) {}

    std::string to_string() const;
    static resource_or_tag_t parse(std::string_view text);

    bool operator==(const resource_or_tag_t&) const = default;

    // serialized as the string form
    void glaze_from(const std::string& text) { *this = parse(text); }
    std::string glaze_to() const { return to_string(); }

    struct glaze {
        using R = resource_or_tag_t;
        static constexpr auto value = glz::custom<&R::glaze_from, &R::glaze_to>;
    };
};

// a "holder set": a single tag ("#a:b"), a single id ("a:b") or a list of ids / tags
struct holder_set_t {
    std::vector<resource_or_tag_t> entries;
    bool is_list = false; // keeps the original shape for round-tripping

    bool is_single_tag() const noexcept { return !is_list && entries.size() == 1 && entries.front().is_tag; }

    // a string for a single non-list entry, an array otherwise
    void from_generic(const glz::generic& j) {
        entries.clear();
        if (j.is_string()) {
            is_list = false;
            entries.push_back(resource_or_tag_t::parse(j.get_string()));
        } else if (j.is_array()) {
            is_list = true;
            for (const auto& element : j.get_array()) {
                if (!element.is_string()) throw jr::error("", "expected a string entry in a holder set");
                entries.push_back(resource_or_tag_t::parse(element.get_string()));
            }
        } else {
            throw jr::error("", "expected a string or an array");
        }
    }
    glz::generic to_generic() const {
        if (!is_list && entries.size() == 1) return glz::generic(entries.front().to_string());
        glz::generic::array_t out;
        for (const auto& entry : entries) out.emplace_back(entry.to_string());
        return glz::generic(std::move(out));
    }

    struct glaze {
        using R = holder_set_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

}
