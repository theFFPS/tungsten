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

[[noreturn]] void fail(const std::string& message) {
    throw jr::error("", message);
}
resource_location_t read_type(const nlohmann::json& j, const char* what) {
    if (!j.is_object()) fail(std::string("expected an object, got ") + j.type_name());
    auto it = j.find("type");
    if (it == j.end()) fail(std::string("missing required key 'type' of ") + what);
    return it->get<resource_location_t>();
}
[[noreturn]] void unknown_type(const resource_location_t& type, const char* what) {
    fail(std::string("unknown ") + what + " type '" + type.to_string() + "'");
}
template<class T> T get_at(const nlohmann::json& j, const char* key) {
    try {
        return j.get<T>();
    } catch (const jr::error& e) {
        throw jr::error(key + (e.m_path.empty() ? "" : "." + e.m_path), e.m_message);
    }
}

}

void to_json(nlohmann::json& j, const dialog_body_t& val) {
    std::visit([&j](const auto& value) { j = value; }, val.m_value);
    j["type"] = val.m_type;
}
void from_json(const nlohmann::json& j, dialog_body_t& val) {
    val.m_type = read_type(j, "dialog body");
    if (val.m_type == resource_location_t("plain_message")) val.m_value = j.get<dialog_body_t::plain_message_t>();
    else if (val.m_type == resource_location_t("item")) val.m_value = j.get<dialog_body_t::item_t>();
    else unknown_type(val.m_type, "dialog body");
}

void to_json(nlohmann::json& j, const dialog_input_control_t::single_option_t::option_t& val) {
    j = { { "id", val.m_id } };
    if (val.m_display.has_value()) j["display"] = *val.m_display;
    if (val.m_initial) j["initial"] = true;
}
void from_json(const nlohmann::json& j, dialog_input_control_t::single_option_t::option_t& val) {
    val = dialog_input_control_t::single_option_t::option_t();
    if (j.is_string()) { // shorthand: just the id
        val.m_id = j.get<std::string>();
        return;
    }
    if (!j.is_object() || !j.contains("id")) fail("a single_option entry requires an 'id'");
    val.m_id = j["id"].get<std::string>();
    if (j.contains("display")) val.m_display = get_at<text_t>(j["display"], "display");
    if (j.contains("initial")) val.m_initial = j["initial"].get<bool>();
}

void to_json(nlohmann::json& j, const dialog_input_control_t& val) {
    std::visit([&j](const auto& value) { j = value; }, val.m_value);
    j["type"] = val.m_type;
    j["key"] = val.m_key;
    j["label"] = val.m_label;
}
void from_json(const nlohmann::json& j, dialog_input_control_t& val) {
    val.m_type = read_type(j, "dialog input control");
    if (!j.contains("key")) fail("missing required key 'key'");
    if (!j.contains("label")) fail("missing required key 'label'");
    val.m_key = j["key"].get<std::string>();
    val.m_label = get_at<text_t>(j["label"], "label");
    if (val.m_type == resource_location_t("text")) val.m_value = j.get<dialog_input_control_t::text_input_t>();
    else if (val.m_type == resource_location_t("boolean")) val.m_value = j.get<dialog_input_control_t::boolean_t>();
    else if (val.m_type == resource_location_t("single_option")) val.m_value = j.get<dialog_input_control_t::single_option_t>();
    else if (val.m_type == resource_location_t("number_range")) val.m_value = j.get<dialog_input_control_t::number_range_t>();
    else unknown_type(val.m_type, "dialog input control");
}

void to_json(nlohmann::json& j, const dialog_ref_t& val) {
    if (val.m_reference.has_value()) j = *val.m_reference;
    else if (val.m_inline) j = *val.m_inline;
    else j = nullptr;
}
void from_json(const nlohmann::json& j, dialog_ref_t& val) {
    val = dialog_ref_t();
    if (j.is_string()) val.m_reference = j.get<resource_or_tag_t>();
    else val.m_inline = std::make_shared<dialog_t>(j.get<dialog_t>());
}

void to_json(nlohmann::json& j, const dialog_t& val) {
    std::visit([&j](const auto& value) { j = value; }, val.m_value);
    j["type"] = val.m_type;
    j["title"] = val.m_title;
    if (val.m_external_title.has_value()) j["external_title"] = *val.m_external_title;
    if (!val.m_body.empty()) j["body"] = val.m_body;
    if (!val.m_inputs.empty()) j["inputs"] = val.m_inputs;
    j["can_close_with_escape"] = val.m_can_close_with_escape;
    j["pause"] = val.m_pause;
    j["after_action"] = val.m_after_action;
}
void from_json(const nlohmann::json& j, dialog_t& val) {
    val = dialog_t();
    val.m_type = read_type(j, "dialog");
    if (!j.contains("title")) fail("missing required key 'title'");
    val.m_title = get_at<text_t>(j["title"], "title");
    if (j.contains("external_title")) val.m_external_title = get_at<text_t>(j["external_title"], "external_title");
    if (j.contains("body")) {
        // a single body element may be given without a list
        if (j["body"].is_array()) val.m_body = get_at<std::vector<dialog_body_t>>(j["body"], "body");
        else val.m_body.push_back(get_at<dialog_body_t>(j["body"], "body"));
    }
    if (j.contains("inputs")) val.m_inputs = get_at<std::vector<dialog_input_control_t>>(j["inputs"], "inputs");
    if (j.contains("can_close_with_escape")) val.m_can_close_with_escape = j["can_close_with_escape"].get<bool>();
    if (j.contains("pause")) val.m_pause = j["pause"].get<bool>();
    if (j.contains("after_action")) val.m_after_action = j["after_action"].get<std::string>();

    if (val.m_type == resource_location_t("notice")) val.m_value = j.get<dialog_t::notice_t>();
    else if (val.m_type == resource_location_t("confirmation")) val.m_value = j.get<dialog_t::confirmation_t>();
    else if (val.m_type == resource_location_t("multi_action")) val.m_value = j.get<dialog_t::multi_action_t>();
    else if (val.m_type == resource_location_t("server_links")) val.m_value = j.get<dialog_t::server_links_t>();
    else if (val.m_type == resource_location_t("dialog_list")) val.m_value = j.get<dialog_t::dialog_list_t>();
    else unknown_type(val.m_type, "dialog");
}

}
