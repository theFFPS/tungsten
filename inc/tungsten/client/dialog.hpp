/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <tungsten/resource/holder_set.hpp>
#include <tungsten/resource/resource_location.hpp>
#include <tungsten/type/text_component.hpp>
#include <tungsten/util/json_reflect.hpp>
#include <tungsten/world/item/item_stack.hpp>

namespace tungsten {

struct dialog_t;

struct dialog_body_t {
    struct plain_message_t {
        text_t contents;
        int width = 200;
    };
    struct item_t {
        item_stack_t item;
        std::optional<std::variant<plain_message_t, text_t>> description;
        bool show_decoration = true;
        bool show_tooltip = true;
        int width = 16;
        int height = 16;
    };

    resource_location_t type;
    std::variant<plain_message_t, item_t> value;

    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = dialog_body_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};
struct dialog_input_control_t {
    struct text_input_t {
        struct multiline_t {
            std::optional<int> max_lines;
            std::optional<int> height;
        };

        int width = 200;
        bool label_visible = true;
        std::string initial = "";
        int max_length = 32;
        std::optional<multiline_t> multiline;
    };
    struct boolean_t {
        bool initial = false;
        std::string on_true = "true";
        std::string on_false = "false";
    };
    struct single_option_t {
        struct option_t {
            std::string id;
            std::optional<text_t> display;
            bool initial = false;

            // an object, or just the id as a string
            void from_generic(const glz::generic& j);
            glz::generic to_generic() const;

            struct glaze {
                using R = option_t;
                static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
            };
        };

        int width = 200;
        std::vector<option_t> options;
        bool label_visible = true;
    };
    struct number_range_t {
        int width = 200;
        std::string label_format = "options.generic_value";
        float start;
        float end;
        std::optional<float> initial;
        std::optional<float> step;
    };

    resource_location_t type;
    std::string key;
    text_t label;
    std::variant<text_input_t, boolean_t, single_option_t, number_range_t> value;

    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = dialog_input_control_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};
struct dialog_action_button_t {
    struct action_t {
        resource_location_t type;

        std::optional<std::string> url;
        std::optional<std::string> command;
        std::optional<int> page;
        std::optional<std::string> value;
        // std::optional<std::variant<resource_location_t, std::shared_ptr<dialog_t>>> dialog;
        // custom
        std::optional<resource_location_t> id; // also dynamic/custom
        std::optional<glz::generic> payload;
        // dynamic/run_command
        std::optional<std::string> template_;
        // also dynamic/custom
        std::optional<glz::generic> additions;
    };

    text_t label;
    std::optional<text_t> tooltip;
    int width = 150;
    std::optional<action_t> action; // TODO! implement dialog actions
};
struct dialog_ref_t {
    std::optional<resource_or_tag_t> reference;
    std::shared_ptr<dialog_t> inline_;

    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = dialog_ref_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

struct dialog_t {
    struct notice_t {
        std::optional<dialog_action_button_t> action;
    };
    struct confirmation_t {
        dialog_action_button_t yes;
        dialog_action_button_t no;
    };
    struct multi_action_t {
        std::vector<dialog_action_button_t> actions;
        std::optional<dialog_action_button_t> exit_action;
        int columns = 2;
    };
    struct server_links_t {
        std::optional<dialog_action_button_t> exit_action;
        int columns = 2;
        int button_width = 150;
    };
    struct dialog_list_t {
        std::variant<resource_or_tag_t, std::vector<dialog_ref_t>> dialogs;
        std::optional<dialog_action_button_t> exit_action;
        int columns = 2;
        int button_width = 150;
    };

    resource_location_t type; // notice || confirmation || multi_action || server_links || dialog_list
    text_t title;
    std::optional<text_t> external_title;
    std::vector<dialog_body_t> body;
    std::vector<dialog_input_control_t> inputs;
    bool can_close_with_escape = true;
    bool pause = true;
    std::string after_action = "close"; // close || none || wait_for_response
    std::variant<notice_t, confirmation_t, multi_action_t, server_links_t, dialog_list_t> value;

    void from_generic(const glz::generic& j);
    glz::generic to_generic() const;

    struct glaze {
        using R = dialog_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

}

// members named like a C++ keyword carry a trailing underscore which is not part of the key
template <> struct glz::meta<tungsten::dialog_action_button_t::action_t> {
    static constexpr std::string_view rename_key(const std::string_view key) { return key.ends_with('_') ? key.substr(0, key.size() - 1) : key; }
};
