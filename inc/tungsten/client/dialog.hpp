#pragma once

#include <tungsten/type/text_component.hpp>
#include <tungsten/resource/resource_location.hpp>
#include <tungsten/world/item/item_stack.hpp>
#include <unordered_map>
#include <variant>

namespace tungsten {

struct dialog_t;
struct dialog_type_t {};
struct dialog_input_control_type_t {};
struct dialog_body_t {
    struct plain_message_t {
        text_t m_contents;
        int m_width = 200;
    };
    struct item_t {
        item_stack_t m_item;
        std::variant<text_t, plain_message_t> m_description;
        bool m_show_decoration = true;
        bool m_show_tooltip = true;
        int m_width = 16;
        int m_height = 16;
    };

    resource_location_t m_type;
    std::variant<plain_message_t, item_t> m_value;
};
struct dialog_input_control_t {
    struct text_input_t { // text
        struct multiline_t {
            std::optional<int> m_max_lines;
            int m_height;
        };

        int m_width = 200;
        bool m_label_visible = true;
        std::string m_initial;
        int m_max_length = 32;
        std::optional<multiline_t> m_multiline;
    };
    struct boolean_t {
        bool m_initial = true;
        std::string m_on_true = "true";
        std::string m_on_false = "false";
    };
    struct single_option_t {
        struct option_t {
            std::string m_id;
            text_t m_display;
            std::optional<bool> m_initial;
        };

        bool m_label_visible = true;
        int m_width = 200;
        std::vector<option_t> m_options;
    };
    struct number_range_t {
        int m_width = 200;
        float m_start;
        float m_end;
        std::optional<float> m_step;
        std::optional<float> m_initial;
    };

    resource_location_t m_type;
    std::string m_key;
    text_t m_label;
    std::variant<text_input_t, boolean_t, single_option_t, number_range_t> m_value;
};
struct dialog_action_t {
    struct open_url_t {
        std::string m_url;
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
    struct dynamic_run_command_t { // dynamic/run_command
        std::string m_template;
    };
    struct dynamic_custom_t { // dynamic/custom
        std::unordered_map<std::string, nlohmann::json> m_additions;
        resource_location_t m_id;
    };

    std::string m_action;
    std::variant<open_url_t, run_command_t, suggest_command_t, change_page_t, copy_to_clipboard_t, show_dialog_t, custom_t, dynamic_run_command_t, dynamic_custom_t> m_value;
};
struct dialog_t {
    struct action_t {
        text_t m_label;
        std::optional<text_t> m_tooltip;
        int m_width = 150;
        dialog_action_t m_action;
    };
    struct notice_t {
        std::optional<action_t> m_action;
    };
    struct confirmation_t {
        action_t m_yes;
        action_t m_no;
    };
    struct multi_action_t {
        std::vector<action_t> m_actions;
        int m_columns = 2;
        action_t m_exit_action;
    };
    struct server_links_t {
        action_t m_exit_action;
        int m_columns = 2;
        int m_button_width = 150;
    };
    struct dialog_list_t {
        std::vector<std::variant<std::string, dialog_t>> m_dialogs;
        action_t m_exit_action;
        int m_columns = 2;
        int m_button_width = 150;
    };

    resource_location_t m_type;
    text_t m_title;
    std::optional<text_t> m_external_title;
    std::vector<dialog_body_t> m_body;
    std::vector<dialog_input_control_t> m_inputs;
    bool m_can_close_with_escape = true;
    bool m_pause = true;
    std::string m_after_action = "close"; // close || none || wait_for_response
    std::variant<notice_t, confirmation_t, multi_action_t, server_links_t, dialog_list_t> m_type_value;

    friend void to_json(nlohmann::json& j, const dialog_t& val) {}
    friend void from_json(const nlohmann::json& j, dialog_t& val) {}
};

}
