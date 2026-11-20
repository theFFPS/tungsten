/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <array>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "game_profile.hpp"
#include <tungsten/resource/resource_location.hpp>
#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

// TODO! implement events
struct text_t {
    struct plain_t { // text
        std::string m_text;
    };
    struct translatable_t {
        std::string m_translate;
        std::optional<std::string> m_fallback;
        std::vector<text_t> m_with;
    };
    struct score_t {
        std::string m_name;
        std::string m_objective;
    };
    struct selector_t {
        std::string m_selector;
        std::optional<std::vector<text_t>> m_separator;
    };
    struct keybind_t {
        std::string m_keybind;
    };
    struct nbt_t {
        std::string m_nbt;
        std::optional<std::string> m_source; // block || entity || storage
        bool m_interpret = false;
        bool m_plain = false;
        std::optional<std::vector<text_t>> m_separator;
        std::optional<std::string> m_entity;
        std::optional<std::string> m_block;
        std::optional<std::string> m_storage;
    };
    struct object_t {
        struct atlas_t {
            std::optional<std::string> m_atlas;
            std::string m_sprite;
        };
        struct player_t {
            std::variant<std::string, game_profile_t> m_player;
            bool m_hat = true;
        };

        std::variant<atlas_t, player_t> m_value;
    };

    std::variant<plain_t, translatable_t, score_t, selector_t, keybind_t, nbt_t, object_t> m_value = plain_t {};

    // style
    std::optional<std::string> m_color; // named color or "#rrggbb"
    std::optional<resource_location_t> m_font;
    std::optional<bool> m_bold;
    std::optional<bool> m_italic;
    std::optional<bool> m_underlined;
    std::optional<bool> m_strikethrough;
    std::optional<bool> m_obfuscated;
    std::optional<std::variant<std::int64_t, std::array<float, 4>>> m_shadow_color;

    // interactivity (kept as validated JSON: the payload depends on `action`)
    std::optional<std::string> m_insertion;
    std::optional<glz::generic> m_click_event;
    std::optional<glz::generic> m_hover_event;

    std::vector<text_t> m_extra;

    static text_t plain(std::string text);
    // flattened visible text for plain / nested plain components (translatable and others are not resolved)
    std::string to_plain_string() const;

    // a string / number / boolean (plain text), a list (first element + extra) or an object
    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = text_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

}
