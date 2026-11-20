/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/client/dialog.hpp>

namespace tungsten {

namespace {

using json = glz::generic;

[[noreturn]] void fail(const std::string& message) {
    throw jr::error("", message);
}
resource_location_t read_type(const json& j, const char* what) {
    if (!j.is_object()) fail("expected an object");
    if (!j.contains("type")) fail(std::string("missing required key 'type' of ") + what);
    return jr::from_generic<resource_location_t>(j["type"]);
}
[[noreturn]] void unknown_type(const resource_location_t& type, const char* what) {
    fail(std::string("unknown ") + what + " type '" + type.to_string() + "'");
}
template<class T> T get_at(const json& j, const char* key) {
    try {
        return jr::from_generic<T>(j);
    } catch (const jr::error& e) {
        throw jr::error(key + (e.m_path.empty() ? "" : "." + e.m_path), e.m_message);
    }
}
// the fields of the type specific struct, as an object
json object_of(const auto& value) {
    return jr::to_generic(value);
}

}

void dialog_body_t::from_generic(const json& j) {
    type = read_type(j, "dialog body");
    if (type == resource_location_t("plain_message")) value = jr::from_generic<plain_message_t>(j);
    else if (type == resource_location_t("item")) value = jr::from_generic<item_t>(j);
    else unknown_type(type, "dialog body");
}
json dialog_body_t::to_generic() const {
    json j = std::visit([](const auto& v) { return object_of(v); }, value);
    j["type"] = type.to_string();
    return j;
}

void dialog_input_control_t::single_option_t::option_t::from_generic(const json& j) {
    *this = option_t();
    if (j.is_string()) { // shorthand: just the id
        id = j.get_string();
        return;
    }
    if (!j.is_object() || !j.contains("id")) fail("a single_option entry requires an 'id'");
    id = jr::from_generic<std::string>(j["id"]);
    if (j.contains("display")) display = get_at<text_t>(j["display"], "display");
    if (j.contains("initial")) initial = jr::from_generic<bool>(j["initial"]);
}
json dialog_input_control_t::single_option_t::option_t::to_generic() const {
    json j = json::object_t{};
    j["id"] = id;
    if (display.has_value()) j["display"] = jr::to_generic(*display);
    if (initial) j["initial"] = true;
    return j;
}

void dialog_input_control_t::from_generic(const json& j) {
    type = read_type(j, "dialog input control");
    if (!j.contains("key")) fail("missing required key 'key'");
    if (!j.contains("label")) fail("missing required key 'label'");
    key = jr::from_generic<std::string>(j["key"]);
    label = get_at<text_t>(j["label"], "label");
    if (type == resource_location_t("text")) value = jr::from_generic<text_input_t>(j);
    else if (type == resource_location_t("boolean")) value = jr::from_generic<boolean_t>(j);
    else if (type == resource_location_t("single_option")) value = jr::from_generic<single_option_t>(j);
    else if (type == resource_location_t("number_range")) value = jr::from_generic<number_range_t>(j);
    else unknown_type(type, "dialog input control");
}
json dialog_input_control_t::to_generic() const {
    json j = std::visit([](const auto& v) { return object_of(v); }, value);
    j["type"] = type.to_string();
    j["key"] = key;
    j["label"] = jr::to_generic(label);
    return j;
}

void dialog_ref_t::from_generic(const json& j) {
    *this = dialog_ref_t();
    if (j.is_string()) reference = resource_or_tag_t::parse(j.get_string());
    else inline_ = std::make_shared<dialog_t>(jr::from_generic<dialog_t>(j));
}
json dialog_ref_t::to_generic() const {
    if (reference.has_value()) return json(reference->to_string());
    if (inline_) return jr::to_generic(*inline_);
    return json(nullptr);
}

void dialog_t::from_generic(const json& j) {
    *this = dialog_t();
    type = read_type(j, "dialog");
    if (!j.contains("title")) fail("missing required key 'title'");
    title = get_at<text_t>(j["title"], "title");
    if (j.contains("external_title")) external_title = get_at<text_t>(j["external_title"], "external_title");
    if (j.contains("body")) {
        // a single body element may be given without a list
        if (j["body"].is_array()) body = get_at<std::vector<dialog_body_t>>(j["body"], "body");
        else body.push_back(get_at<dialog_body_t>(j["body"], "body"));
    }
    if (j.contains("inputs")) inputs = get_at<std::vector<dialog_input_control_t>>(j["inputs"], "inputs");
    if (j.contains("can_close_with_escape")) can_close_with_escape = jr::from_generic<bool>(j["can_close_with_escape"]);
    if (j.contains("pause")) pause = jr::from_generic<bool>(j["pause"]);
    if (j.contains("after_action")) after_action = jr::from_generic<std::string>(j["after_action"]);

    // the common keys were read above, the type specific struct only knows its own keys
    if (type == resource_location_t("notice")) value = jr::from_generic<notice_t>(j);
    else if (type == resource_location_t("confirmation")) value = jr::from_generic<confirmation_t>(j);
    else if (type == resource_location_t("multi_action")) value = jr::from_generic<multi_action_t>(j);
    else if (type == resource_location_t("server_links")) value = jr::from_generic<server_links_t>(j);
    else if (type == resource_location_t("dialog_list")) value = jr::from_generic<dialog_list_t>(j);
    else unknown_type(type, "dialog");
}
json dialog_t::to_generic() const {
    json j = std::visit([](const auto& v) { return object_of(v); }, value);
    j["type"] = type.to_string();
    j["title"] = jr::to_generic(title);
    if (external_title.has_value()) j["external_title"] = jr::to_generic(*external_title);
    if (!body.empty()) j["body"] = jr::to_generic(body);
    if (!inputs.empty()) j["inputs"] = jr::to_generic(inputs);
    j["can_close_with_escape"] = can_close_with_escape;
    j["pause"] = pause;
    j["after_action"] = after_action;
    return j;
}

}
