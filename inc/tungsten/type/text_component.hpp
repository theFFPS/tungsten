/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <array>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "game_profile.hpp"
#include <tungsten/world/item/item_stack.hpp>
#include <tungsten/resource/resource_location.hpp>

namespace tungsten {

struct text_t;
struct dialog_t;

struct text_component_click_event_t {
    struct open_url_t {
        std::string m_url;
    };
    struct open_file_t {
        std::string m_path;
    };
    struct run_command_t {
        std::string m_command;
    };
    struct suggest_command_t {
        std::string m_command;
    };
    struct change_page_t {
        int m_page;
    };
    struct copy_to_clipboard_t {
        std::string m_value;
    };
    struct show_dialog_t {
        std::variant<resource_location_t, std::vector<dialog_t>> m_dialog;
    };
    struct custom_t {
        std::string m_id;
        std::optional<nlohmann::json> m_payload;
    };

    std::string m_action;
    std::variant<open_url_t, open_file_t, run_command_t, suggest_command_t, change_page_t, copy_to_clipboard_t, show_dialog_t, custom_t> m_value;
};
struct text_component_hover_event_t {
    struct show_text_t {
        std::variant<std::string, std::vector<text_t>> m_value;
    };
    using show_item_t = item_stack_t;
    struct show_entity_t {
        std::variant<std::string, std::vector<text_t>> m_name;
        resource_location_t m_id;
        std::variant<std::array<int, 4>> m_uuid;
    };

    std::string m_action;
    std::variant<show_text_t, show_item_t, show_entity_t> m_value;
};

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
        struct score_internal_t {
            std::string m_name;
            std::string m_objective;
        } m_score;
    };
    struct selector_t {
        std::string m_selector;
        std::optional<std::variant<std::string, std::vector<text_t>>> m_separator;
    };
    struct keybind_t {
        std::string m_keybind;
    };
    struct nbt_t {
        std::optional<std::string> m_source;
        std::string m_nbt;
        bool m_interpret = false;
        bool m_plain = false;
        std::optional<std::variant<std::string, std::vector<text_t>>> m_separator;
        std::optional<std::string> m_entity;
        std::optional<std::string> m_block;
        std::optional<std::string> m_storage;
    };
    struct object_t {
        struct atlas_t {
            std::optional<std::string> m_atlas;
            std::string m_sprint;
        };
        struct player_t {
            std::variant<std::string, game_profile_t> m_player;
            bool m_hat = true;
        };

        resource_location_t m_object = resource_location_t("atlas");
        std::variant<atlas_t, player_t> m_value;
    };

    resource_location_t m_type = resource_location_t("text");
    std::variant<plain_t, translatable_t, score_t, selector_t, keybind_t, nbt_t, object_t> m_value;

    // style
    std::optional<std::string> m_color;
    resource_location_t m_font = resource_location_t("default");
    bool m_bold = false;
    bool m_italic = false;
    bool m_underlined = false;
    bool m_strikethrough = false;
    bool m_obfuscated = false;
    std::optional<std::variant<int, std::array<float, 4>>> m_shadow_color;

    // interactivity
    std::optional<std::string> insertion;
    std::optional<text_component_click_event_t> click_event;
    std::optional<text_component_hover_event_t> hover_event;

    std::vector<text_t> extra;

    friend void to_json(nlohmann::json& j, const text_t& val) {}
    friend void from_json(const nlohmann::json& j, text_t& val) {}
};

}
