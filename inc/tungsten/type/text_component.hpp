#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "game_profile.hpp"

namespace tungsten {

struct text_t;

struct text_component_score_t {
    std::string name;
    std::string objective;
};
struct text_component_click_event_t {
    std::string action;
    std::optional<std::string> url; // action == 'open_url'
    std::optional<std::string> path; // action == 'open_file'
    std::optional<std::string> command; // action == 'run_command' || action == 'suggest_command'
    std::optional<int> page; // action == 'change_page'
    std::optional<std::string> value; // action == 'copy_to_clipboard'
    // todo: show_dialog

    // action == 'custom'
    std::optional<std::string> id;
    // std::optional<glz::generic_json> payload;
};
struct text_component_hover_event_t {
    std::string action;
    std::optional<std::unique_ptr<std::variant<std::string, std::vector<text_t>, text_t>>> value; // action == 'show_text'

    // action == 'show_item'
    std::optional<std::string> id; // also for action == 'show_entity'
    std::optional<int> count;
    // todo: components

    // action == 'show_entity'
    // id
    std::optional<std::unique_ptr<std::variant<std::string, std::vector<text_t>, text_t>>> name;
    std::optional<std::variant<std::string, std::array<int, 4>>> uuid;
};

struct text_t {
    // type == 'text'
    std::optional<std::string> text;

    // type == 'translatable'
    std::optional<std::string> translate;
    std::optional<std::string> fallback;
    std::optional<std::vector<text_t>> with;

    // type == 'score'
    std::optional<text_component_score_t> score;

    // type == 'selector'
    std::optional<std::string> selector;
    std::optional<std::unique_ptr<std::variant<std::string, std::vector<text_t>, text_t>>> separator; // also for type == 'nbt'

    // type == 'keybind'
    std::optional<std::string> keybind;

    // type == 'nbt'
    std::optional<std::string> source;
    std::optional<std::string> nbt;
    std::optional<bool> interpret;
    std::optional<bool> plain;
    // separator was defined earlier
    std::optional<std::string> entity;
    std::optional<std::string> block;
    std::optional<std::string> storage;

    // type == 'object'
    /// object == 'atlas'
    std::optional<std::string> atlas;
    std::optional<std::string> sprite;
    /// object == 'player'
    std::optional<bool> hat;
    std::optional<std::variant<std::string, game_profile_t>> player;

    // style
    std::optional<std::string> color;
    std::string font = "minecraft:default";
    bool bold = false;
    bool italic = false;
    bool underlined = false;
    bool strikethrough = false;
    bool obfuscated = false;
    std::optional<std::variant<int, std::array<float, 4>>> shadow_color;

    // interactivity
    std::optional<std::string> insertion;
    std::optional<text_component_click_event_t> click_event;
    std::optional<text_component_hover_event_t> hover_event;

    // extra
    std::optional<std::vector<text_t>> extra;
};

}
