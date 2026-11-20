/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/type/text_component.hpp>
#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

namespace {

using json = nlohmann::json;

[[noreturn]] void fail(const std::string& message) {
    throw jr::error("", message);
}

template<class T> std::optional<T> opt(const json& j, const char* key) {
    auto it = j.find(key);
    if (it == j.end() || it->is_null()) return std::nullopt;
    try {
        return it->get<T>();
    } catch (const jr::error& e) {
        throw jr::error(std::string(key) + (e.m_path.empty() ? "" : "." + e.m_path), e.m_message);
    } catch (const std::exception& e) {
        throw jr::error(key, e.what());
    }
}
template<class T> T req(const json& j, const char* key) {
    auto value = opt<T>(j, key);
    if (!value.has_value()) fail(std::string("missing required key '") + key + "'");
    return std::move(*value);
}

// a separator may be a single component or a list; always held as a list
std::optional<std::vector<text_t>> parse_components(const json& j, const char* key) {
    auto it = j.find(key);
    if (it == j.end() || it->is_null()) return std::nullopt;
    if (it->is_array()) return it->get<std::vector<text_t>>();
    return std::vector<text_t> { it->get<text_t>() };
}

json scalar_to_string(const json& j) {
    if (j.is_string()) return j;
    if (j.is_boolean()) return j.get<bool>() ? "true" : "false";
    return j.dump();
}

void parse_object(const json& j, text_t& out) {
    std::optional<std::string> type;
    if (j.contains("type")) type = j["type"].get<std::string>();
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
        if (has("text")) value.m_text = scalar_to_string(j["text"]).get<std::string>();
        out.m_value = std::move(value);
    } else if (*type == "translatable") {
        text_t::translatable_t value;
        value.m_translate = req<std::string>(j, "translate");
        value.m_fallback = opt<std::string>(j, "fallback");
        auto it = j.find("with");
        if (it != j.end()) {
            if (!it->is_array()) fail("'with' must be an array");
            for (const auto& arg : *it) {
                if (arg.is_string() || arg.is_number() || arg.is_boolean()) {
                    text_t plain;
                    plain.m_value = text_t::plain_t { scalar_to_string(arg).get<std::string>() };
                    value.m_with.push_back(std::move(plain));
                } else value.m_with.push_back(arg.get<text_t>());
            }
        }
        out.m_value = std::move(value);
    } else if (*type == "score") {
        const json& score = j.contains("score") ? j["score"] : j;
        if (!score.is_object()) fail("'score' must be an object");
        out.m_value = text_t::score_t { req<std::string>(score, "name"), req<std::string>(score, "objective") };
    } else if (*type == "selector") {
        text_t::selector_t value;
        value.m_selector = req<std::string>(j, "selector");
        value.m_separator = parse_components(j, "separator");
        out.m_value = std::move(value);
    } else if (*type == "keybind") {
        out.m_value = text_t::keybind_t { req<std::string>(j, "keybind") };
    } else if (*type == "nbt") {
        text_t::nbt_t value;
        value.m_nbt = req<std::string>(j, "nbt");
        value.m_source = opt<std::string>(j, "source");
        value.m_interpret = opt<bool>(j, "interpret").value_or(false);
        value.m_plain = opt<bool>(j, "plain").value_or(false);
        value.m_separator = parse_components(j, "separator");
        value.m_entity = opt<std::string>(j, "entity");
        value.m_block = opt<std::string>(j, "block");
        value.m_storage = opt<std::string>(j, "storage");
        out.m_value = std::move(value);
    } else if (*type == "object") {
        text_t::object_t value;
        if (has("player")) {
            text_t::object_t::player_t player;
            const json& p = j["player"];
            if (p.is_string()) player.m_player = p.get<std::string>();
            else player.m_player = p.get<game_profile_t>();
            player.m_hat = opt<bool>(j, "hat").value_or(true);
            value.m_value = std::move(player);
        } else {
            text_t::object_t::atlas_t atlas;
            atlas.m_atlas = opt<std::string>(j, "atlas");
            atlas.m_sprite = req<std::string>(j, "sprite");
            value.m_value = std::move(atlas);
        }
        out.m_value = std::move(value);
    } else {
        fail("unknown text component type '" + *type + "'");
    }

    out.m_color = opt<std::string>(j, "color");
    if (auto font = opt<std::string>(j, "font")) out.m_font = resource_location_t(*font);
    out.m_bold = opt<bool>(j, "bold");
    out.m_italic = opt<bool>(j, "italic");
    out.m_underlined = opt<bool>(j, "underlined");
    out.m_strikethrough = opt<bool>(j, "strikethrough");
    out.m_obfuscated = opt<bool>(j, "obfuscated");
    if (has("shadow_color") && !j["shadow_color"].is_null()) {
        const json& sc = j["shadow_color"];
        if (sc.is_number_integer()) out.m_shadow_color = sc.get<std::int64_t>();
        else if (sc.is_array() && sc.size() == 4) out.m_shadow_color = sc.get<std::array<float, 4>>();
        else fail("'shadow_color' must be an integer or an array of four floats");
    }
    out.m_insertion = opt<std::string>(j, "insertion");
    out.m_click_event = opt<json>(j, "click_event");
    out.m_hover_event = opt<json>(j, "hover_event");
    if (has("extra")) {
        if (!j["extra"].is_array()) fail("'extra' must be an array");
        out.m_extra = j["extra"].get<std::vector<text_t>>();
    }
}

void write_components(json& j, const char* key, const std::optional<std::vector<text_t>>& value) {
    if (!value.has_value()) return;
    if (value->size() == 1) j[key] = value->front();
    else j[key] = *value;
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

void from_json(const json& j, text_t& val) {
    val = text_t();
    if (j.is_string() || j.is_number() || j.is_boolean()) {
        val.m_value = text_t::plain_t { scalar_to_string(j).get<std::string>() };
    } else if (j.is_array()) {
        if (j.empty()) fail("a text component list must not be empty");
        // [first, ...rest]: the rest becomes `extra` of the first element
        val = j[0].get<text_t>();
        for (std::size_t i = 1; i < j.size(); ++i) val.m_extra.push_back(j[i].get<text_t>());
    } else if (j.is_object()) {
        parse_object(j, val);
    } else {
        fail(std::string("expected a text component (string, list or object), got ") + j.type_name());
    }
}
void to_json(json& j, const text_t& val) {
    j = json::object();
    std::visit([&j](const auto& v) {
        using V = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<V, text_t::plain_t>) {
            j["text"] = v.m_text;
        } else if constexpr (std::is_same_v<V, text_t::translatable_t>) {
            j["translate"] = v.m_translate;
            if (v.m_fallback) j["fallback"] = *v.m_fallback;
            if (!v.m_with.empty()) j["with"] = v.m_with;
        } else if constexpr (std::is_same_v<V, text_t::score_t>) {
            j["score"] = { { "name", v.m_name }, { "objective", v.m_objective } };
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
                    std::visit([&j](const auto& player) { j["player"] = player; }, o.m_player);
                    if (!o.m_hat) j["hat"] = false;
                }
            }, v.m_value);
        }
    }, val.m_value);
    if (val.m_color) j["color"] = *val.m_color;
    if (val.m_font) j["font"] = *val.m_font;
    if (val.m_bold) j["bold"] = *val.m_bold;
    if (val.m_italic) j["italic"] = *val.m_italic;
    if (val.m_underlined) j["underlined"] = *val.m_underlined;
    if (val.m_strikethrough) j["strikethrough"] = *val.m_strikethrough;
    if (val.m_obfuscated) j["obfuscated"] = *val.m_obfuscated;
    if (val.m_shadow_color) std::visit([&j](const auto& c) { j["shadow_color"] = c; }, *val.m_shadow_color);
    if (val.m_insertion) j["insertion"] = *val.m_insertion;
    if (val.m_click_event) j["click_event"] = *val.m_click_event;
    if (val.m_hover_event) j["hover_event"] = *val.m_hover_event;
    if (!val.m_extra.empty()) j["extra"] = val.m_extra;
}

}
