/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <cmath>
#include <tungsten/type/text_component.hpp>
#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

namespace {

using json = glz::generic;

[[noreturn]] void fail(const std::string& message) {
    throw jr::error("", message);
}

// the member `key` of an object, or nullptr when absent / null
const json* find(const json& j, const char* key) {
    if (!j.is_object() || !j.contains(key)) return nullptr;
    const json& value = j[key];
    return value.is_null() ? nullptr : &value;
}

std::string as_string(const json& v, const char* key) {
    if (!v.is_string()) throw jr::error(key, "expected a string");
    return v.get_string();
}
bool as_bool(const json& v, const char* key) {
    if (!v.is_boolean()) throw jr::error(key, "expected a boolean");
    return v.get_boolean();
}
std::optional<std::string> opt_string(const json& j, const char* key) {
    if (const json* v = find(j, key)) return as_string(*v, key);
    return std::nullopt;
}
std::optional<bool> opt_bool(const json& j, const char* key) {
    if (const json* v = find(j, key)) return as_bool(*v, key);
    return std::nullopt;
}
std::string req_string(const json& j, const char* key) {
    auto value = opt_string(j, key);
    if (!value.has_value()) fail(std::string("missing required key '") + key + "'");
    return std::move(*value);
}

text_t parse_component(const json& j, const char* key) {
    try {
        text_t result;
        result.from_generic(j);
        return result;
    } catch (const jr::error& e) {
        throw jr::error(std::string(key) + (e.m_path.empty() ? "" : (e.m_path.starts_with('[') ? "" : ".") + e.m_path), e.m_message);
    }
}

// a separator may be a single component or a list; always held as a list
std::optional<std::vector<text_t>> parse_components(const json& j, const char* key) {
    const json* v = find(j, key);
    if (!v) return std::nullopt;
    std::vector<text_t> result;
    if (v->is_array()) {
        for (const auto& element : v->get_array()) result.push_back(parse_component(element, key));
    } else {
        result.push_back(parse_component(*v, key));
    }
    return result;
}

std::string scalar_to_string(const json& j) {
    if (j.is_string()) return j.get_string();
    if (j.is_boolean()) return j.get_boolean() ? "true" : "false";
    std::string text;
    (void)glz::write_json(j, text);
    return text;
}

bool is_integer(const json& j) {
    return j.is_number() && std::floor(j.get_number()) == j.get_number();
}

void parse_object(const json& j, text_t& out) {
    std::optional<std::string> type;
    if (j.contains("type")) type = as_string(j["type"], "type");
    if (type.has_value() && type->starts_with("minecraft:")) type->erase(0, 10);

    auto has = [&j](const char* key) { return j.contains(key); };
    // an absent type is inferred from the content keys, like the game does
    if (!type.has_value()) {
        if (has("text")) type = "text";
        else if (has("translate")) type = "translatable";
        else if (has("score")) type = "score";
        else if (has("selector")) type = "selector";
        else if (has("keybind")) type = "keybind";
        else if (has("nbt")) type = "nbt";
        else if (has("object") || has("atlas") || has("sprite") || has("player")) type = "object";
        else type = "text"; // style only / empty component
    }

    if (*type == "text") {
        text_t::plain_t value;
        if (has("text")) value.m_text = scalar_to_string(j["text"]);
        out.m_value = std::move(value);
    } else if (*type == "translatable") {
        text_t::translatable_t value;
        value.m_translate = req_string(j, "translate");
        value.m_fallback = opt_string(j, "fallback");
        if (has("with")) {
            const json& with = j["with"];
            if (!with.is_array()) fail("'with' must be an array");
            for (const auto& arg : with.get_array()) {
                if (arg.is_string() || arg.is_number() || arg.is_boolean()) {
                    text_t plain;
                    plain.m_value = text_t::plain_t { scalar_to_string(arg) };
                    value.m_with.push_back(std::move(plain));
                } else value.m_with.push_back(parse_component(arg, "with"));
            }
        }
        out.m_value = std::move(value);
    } else if (*type == "score") {
        const json& score = j.contains("score") ? j["score"] : j;
        if (!score.is_object()) fail("'score' must be an object");
        out.m_value = text_t::score_t { req_string(score, "name"), req_string(score, "objective") };
    } else if (*type == "selector") {
        text_t::selector_t value;
        value.m_selector = req_string(j, "selector");
        value.m_separator = parse_components(j, "separator");
        out.m_value = std::move(value);
    } else if (*type == "keybind") {
        out.m_value = text_t::keybind_t { req_string(j, "keybind") };
    } else if (*type == "nbt") {
        text_t::nbt_t value;
        value.m_nbt = req_string(j, "nbt");
        value.m_source = opt_string(j, "source");
        value.m_interpret = opt_bool(j, "interpret").value_or(false);
        value.m_plain = opt_bool(j, "plain").value_or(false);
        value.m_separator = parse_components(j, "separator");
        value.m_entity = opt_string(j, "entity");
        value.m_block = opt_string(j, "block");
        value.m_storage = opt_string(j, "storage");
        out.m_value = std::move(value);
    } else if (*type == "object") {
        text_t::object_t value;
        if (has("player")) {
            text_t::object_t::player_t player;
            const json& p = j["player"];
            if (p.is_string()) player.m_player = p.get_string();
            else player.m_player = jr::from_generic<game_profile_t>(p);
            player.m_hat = opt_bool(j, "hat").value_or(true);
            value.m_value = std::move(player);
        } else {
            text_t::object_t::atlas_t atlas;
            atlas.m_atlas = opt_string(j, "atlas");
            atlas.m_sprite = req_string(j, "sprite");
            value.m_value = std::move(atlas);
        }
        out.m_value = std::move(value);
    } else {
        fail("unknown text component type '" + *type + "'");
    }

    out.m_color = opt_string(j, "color");
    if (auto font = opt_string(j, "font")) out.m_font = resource_location_t(*font);
    out.m_bold = opt_bool(j, "bold");
    out.m_italic = opt_bool(j, "italic");
    out.m_underlined = opt_bool(j, "underlined");
    out.m_strikethrough = opt_bool(j, "strikethrough");
    out.m_obfuscated = opt_bool(j, "obfuscated");
    if (const json* sc = find(j, "shadow_color")) {
        if (is_integer(*sc)) out.m_shadow_color = static_cast<std::int64_t>(sc->get_number());
        else if (sc->is_array() && sc->get_array().size() == 4) {
            std::array<float, 4> color {};
            for (std::size_t i = 0; i < 4; ++i) {
                const json& channel = sc->get_array()[i];
                if (!channel.is_number()) fail("'shadow_color' must be an integer or an array of four floats");
                color[i] = static_cast<float>(channel.get_number());
            }
            out.m_shadow_color = color;
        } else fail("'shadow_color' must be an integer or an array of four floats");
    }
    out.m_insertion = opt_string(j, "insertion");
    if (const json* v = find(j, "click_event")) out.m_click_event = *v;
    if (const json* v = find(j, "hover_event")) out.m_hover_event = *v;
    if (has("extra")) {
        const json& extra = j["extra"];
        if (!extra.is_array()) fail("'extra' must be an array");
        for (const auto& child : extra.get_array()) out.m_extra.push_back(parse_component(child, "extra"));
    }
}

json components_to_generic(const std::vector<text_t>& value) {
    json::array_t array;
    for (const auto& child : value) array.push_back(child.to_generic());
    return json(std::move(array));
}
void write_components(json& j, const char* key, const std::optional<std::vector<text_t>>& value) {
    if (!value.has_value()) return;
    if (value->size() == 1) j[key] = value->front().to_generic();
    else j[key] = components_to_generic(*value);
}

}

text_t text_t::plain(std::string text) {
    text_t result;
    result.m_value = plain_t { std::move(text) };
    return result;
}
std::string text_t::to_plain_string() const {
    std::string result;
    if (const auto* p = std::get_if<plain_t>(&m_value)) result = p->m_text;
    else if (const auto* t = std::get_if<translatable_t>(&m_value)) result = t->m_fallback.value_or(t->m_translate);
    for (const auto& child : m_extra) result += child.to_plain_string();
    return result;
}

void text_t::from_generic(const json& j) {
    *this = text_t();
    if (j.is_string() || j.is_number() || j.is_boolean()) {
        m_value = plain_t { scalar_to_string(j) };
    } else if (j.is_array()) {
        const auto& list = j.get_array();
        if (list.empty()) fail("a text component list must not be empty");
        // [first, ...rest]: the rest becomes `extra` of the first element
        from_generic(list[0]);
        for (std::size_t i = 1; i < list.size(); ++i) {
            text_t child;
            child.from_generic(list[i]);
            m_extra.push_back(std::move(child));
        }
    } else if (j.is_object()) {
        parse_object(j, *this);
    } else {
        fail("expected a text component (string, list or object)");
    }
}
json text_t::to_generic() const {
    json j = json::object_t{};
    std::visit([&j](const auto& v) {
        using V = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<V, text_t::plain_t>) {
            j["text"] = v.m_text;
        } else if constexpr (std::is_same_v<V, text_t::translatable_t>) {
            j["translate"] = v.m_translate;
            if (v.m_fallback) j["fallback"] = *v.m_fallback;
            if (!v.m_with.empty()) j["with"] = components_to_generic(v.m_with);
        } else if constexpr (std::is_same_v<V, text_t::score_t>) {
            json score = json::object_t{};
            score["name"] = v.m_name;
            score["objective"] = v.m_objective;
            j["score"] = std::move(score);
        } else if constexpr (std::is_same_v<V, text_t::selector_t>) {
            j["selector"] = v.m_selector;
            write_components(j, "separator", v.m_separator);
        } else if constexpr (std::is_same_v<V, text_t::keybind_t>) {
            j["keybind"] = v.m_keybind;
        } else if constexpr (std::is_same_v<V, text_t::nbt_t>) {
            j["nbt"] = v.m_nbt;
            if (v.m_source) j["source"] = *v.m_source;
            if (v.m_interpret) j["interpret"] = true;
            if (v.m_plain) j["plain"] = true;
            write_components(j, "separator", v.m_separator);
            if (v.m_entity) j["entity"] = *v.m_entity;
            if (v.m_block) j["block"] = *v.m_block;
            if (v.m_storage) j["storage"] = *v.m_storage;
        } else if constexpr (std::is_same_v<V, text_t::object_t>) {
            j["type"] = "object";
            std::visit([&j](const auto& o) {
                using O = std::decay_t<decltype(o)>;
                if constexpr (std::is_same_v<O, text_t::object_t::atlas_t>) {
                    if (o.m_atlas) j["atlas"] = *o.m_atlas;
                    j["sprite"] = o.m_sprite;
                } else {
                    std::visit([&j](const auto& player) {
                        if constexpr (std::is_same_v<std::decay_t<decltype(player)>, std::string>) j["player"] = player;
                        else j["player"] = jr::to_generic(player);
                    }, o.m_player);
                    if (!o.m_hat) j["hat"] = false;
                }
            }, v.m_value);
        }
    }, m_value);
    if (m_color) j["color"] = *m_color;
    if (m_font) j["font"] = m_font->to_string();
    if (m_bold) j["bold"] = *m_bold;
    if (m_italic) j["italic"] = *m_italic;
    if (m_underlined) j["underlined"] = *m_underlined;
    if (m_strikethrough) j["strikethrough"] = *m_strikethrough;
    if (m_obfuscated) j["obfuscated"] = *m_obfuscated;
    if (m_shadow_color) std::visit([&j](const auto& c) {
        if constexpr (std::is_same_v<std::decay_t<decltype(c)>, std::int64_t>) j["shadow_color"] = static_cast<double>(c);
        else {
            json::array_t channels;
            for (float channel : c) channels.emplace_back(static_cast<double>(channel));
            j["shadow_color"] = json(std::move(channels));
        }
    }, *m_shadow_color);
    if (m_insertion) j["insertion"] = *m_insertion;
    if (m_click_event) j["click_event"] = *m_click_event;
    if (m_hover_event) j["hover_event"] = *m_hover_event;
    if (!m_extra.empty()) j["extra"] = components_to_generic(m_extra);
    return j;
}

}
