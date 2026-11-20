/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/datapack/pack_meta.hpp>

#include <cmath>
#include <stdexcept>
#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

namespace {

using json = glz::generic;

[[noreturn]] void fail(const std::string& message) {
    throw std::runtime_error("pack_meta_t error: " + message);
}

bool is_integer(const json& j) {
    return j.is_number() && std::floor(j.get_number()) == j.get_number();
}
std::int32_t as_int(const json& j) {
    return static_cast<std::int32_t>(j.get_number());
}
std::size_t size_of(const json& j) {
    return j.get_array().size();
}

std::int32_t read_major(const json& j, const char* what) {
    if (!is_integer(j)) fail(std::string(what) + " must be an integer");
    return as_int(j);
}

// legacy 'supported_formats' / overlay 'formats': 42 | [42, 45] | { "min_inclusive": 42, "max_inclusive": 45 }
std::pair<std::int32_t, std::int32_t> parse_legacy_range(const json& j, const char* what) {
    if (is_integer(j)) return { as_int(j), as_int(j) };
    if (j.is_array()) {
        if (size_of(j) == 1) return { read_major(j[0], what), read_major(j[0], what) };
        if (size_of(j) == 2) return { read_major(j[0], what), read_major(j[1], what) };
        fail(std::string(what) + " array must hold one or two integers");
    }
    if (j.is_object()) {
        if (!j.contains("min_inclusive") || !j.contains("max_inclusive")) {
            fail(std::string(what) + " object requires 'min_inclusive' and 'max_inclusive'");
        }
        return { read_major(j["min_inclusive"], what), read_major(j["max_inclusive"], what) };
    }
    fail(std::string(what) + " has an unsupported type");
}

// shared by the 'pack' section and overlay entries
pack_version_range_t parse_range(const json& j, const char* legacy_key, const std::optional<std::int32_t>& pack_format, const std::string& where) {
    bool has_min = j.contains("min_format");
    bool has_max = j.contains("max_format");
    if (has_min != has_max) fail(where + ": 'min_format' and 'max_format' must be specified together");
    pack_version_range_t range;
    if (has_min) {
        range.m_min = pack_version_t::parse_min(j["min_format"]);
        range.m_max = pack_version_t::parse_max(j["max_format"]);
    } else if (legacy_key != nullptr && j.contains(legacy_key)) {
        auto [lo, hi] = parse_legacy_range(j[legacy_key], legacy_key);
        range.m_min = { lo, 0 };
        range.m_max = { hi, pack_version_t::any_minor };
    } else if (pack_format.has_value()) {
        range.m_min = { *pack_format, 0 };
        range.m_max = { *pack_format, pack_version_t::any_minor };
    } else {
        fail(where + ": neither 'min_format'/'max_format' nor a legacy format is specified");
    }
    if (range.m_min > range.m_max) fail(where + ": 'min_format' is greater than 'max_format'");
    return range;
}

bool valid_overlay_directory(std::string_view name) {
    if (name.empty()) return false;
    for (char c : name) {
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) return false;
    }
    return name != "." && name != "..";
}

std::regex compile_regex(const std::string& pattern, const char* what) {
    try {
        return std::regex(pattern, std::regex::ECMAScript);
    } catch (const std::regex_error& e) {
        fail(std::string("invalid regular expression in filter ") + what + " '" + pattern + "': " + e.what());
    }
}

}

std::string pack_version_t::to_string() const {
    return std::to_string(m_major) + "." + (m_minor == any_minor ? "*" : std::to_string(m_minor));
}
pack_version_t pack_version_t::parse_min(const json& j) {
    if (is_integer(j)) return { as_int(j), 0 };
    if (j.is_array() && size_of(j) == 1) return { read_major(j[0], "min_format"), 0 };
    if (j.is_array() && size_of(j) == 2) return { read_major(j[0], "min_format"), read_major(j[1], "min_format") };
    fail("'min_format' must be an integer or an array of one or two integers");
}
pack_version_t pack_version_t::parse_max(const json& j) {
    if (is_integer(j)) return { as_int(j), any_minor };
    if (j.is_array() && size_of(j) == 1) return { read_major(j[0], "max_format"), any_minor };
    if (j.is_array() && size_of(j) == 2) return { read_major(j[0], "max_format"), read_major(j[1], "max_format") };
    fail("'max_format' must be an integer or an array of one or two integers");
}

bool pack_filter_pattern_t::matches(std::string_view namespace_s, std::string_view path) const {
    // java's Pattern.asPredicate() uses find(), so partial matches count
    if (m_namespace.has_value() && !std::regex_search(namespace_s.begin(), namespace_s.end(), m_namespace_regex)) return false;
    if (m_path.has_value() && !std::regex_search(path.begin(), path.end(), m_path_regex)) return false;
    return true;
}

pack_meta_t pack_meta_t::parse(std::string_view text) {
    if (text.starts_with("\xEF\xBB\xBF")) text.remove_prefix(3);
    json j;
    // like the game: comments are tolerated
    if (auto ec = glz::read<glz::opts{.comments = true}>(j, text)) {
        fail(std::string("pack.mcmeta is not valid JSON: ") + glz::format_error(ec, text));
    }
    return from_json(j);
}
pack_meta_t pack_meta_t::from_json(const json& j) {
    if (!j.is_object()) fail("pack.mcmeta root must be an object");
    if (!j.contains("pack") || !j["pack"].is_object()) fail("missing required section 'pack'");
    const json& pack = j["pack"];

    pack_meta_t meta;
    meta.m_raw = j;
    if (!pack.contains("description")) fail("missing required key 'pack.description'");
    meta.m_description = pack["description"];
    if (pack.contains("pack_format")) meta.m_pack_format = read_major(pack["pack_format"], "pack_format");
    meta.m_range = parse_range(pack, "supported_formats", meta.m_pack_format, "'pack'");

    if (j.contains("features")) {
        const json& features = j["features"];
        if (!features.is_object()) fail("'features' must be an object");
        if (features.contains("enabled")) {
            if (!features["enabled"].is_array()) fail("'features.enabled' must be an array");
            for (const auto& flag : features["enabled"].get_array()) {
                if (!flag.is_string() || !resource_location_t::is_valid(flag.get_string())) {
                    fail("'features.enabled' contains an invalid feature flag");
                }
                meta.m_enabled_features.emplace_back(flag.get_string());
            }
        }
    }
    if (j.contains("filter")) {
        const json& filter = j["filter"];
        if (!filter.is_object() || !filter.contains("block") || !filter["block"].is_array()) {
            fail("'filter' must be an object with a 'block' array");
        }
        for (const auto& entry : filter["block"].get_array()) {
            if (!entry.is_object()) fail("'filter.block' entries must be objects");
            pack_filter_pattern_t pattern;
            if (entry.contains("namespace")) {
                pattern.m_namespace = entry["namespace"].get_string();
                pattern.m_namespace_regex = compile_regex(*pattern.m_namespace, "namespace");
            }
            if (entry.contains("path")) {
                pattern.m_path = entry["path"].get_string();
                pattern.m_path_regex = compile_regex(*pattern.m_path, "path");
            }
            meta.m_filter_block.push_back(std::move(pattern));
        }
    }
    if (j.contains("overlays")) {
        const json& overlays = j["overlays"];
        if (!overlays.is_object() || !overlays.contains("entries") || !overlays["entries"].is_array()) {
            fail("'overlays' must be an object with an 'entries' array");
        }
        for (const auto& entry : overlays["entries"].get_array()) {
            if (!entry.is_object() || !entry.contains("directory") || !entry["directory"].is_string()) {
                fail("overlay entries require a string 'directory'");
            }
            pack_overlay_t overlay;
            overlay.m_directory = entry["directory"].get_string();
            if (!valid_overlay_directory(overlay.m_directory)) {
                fail("overlay directory '" + overlay.m_directory + "' contains invalid characters (allowed: a-z 0-9 _ - .)");
            }
            overlay.m_range = parse_range(entry, "formats", std::nullopt, "overlay '" + overlay.m_directory + "'");
            meta.m_overlays.push_back(std::move(overlay));
        }
    }
    return meta;
}

text_t pack_meta_t::description_text() const {
    try {
        return jr::from_generic<text_t>(m_description);
    } catch (const std::exception&) {
        std::string dumped;
        (void)glz::write_json(m_description, dumped);
        text_t fallback;
        fallback.m_value = text_t::plain_t { dumped };
        return fallback;
    }
}
bool pack_meta_t::is_filtered(std::string_view namespace_s, std::string_view path) const {
    for (const auto& pattern : m_filter_block) {
        if (pattern.matches(namespace_s, path)) return true;
    }
    return false;
}
std::vector<std::string> pack_meta_t::active_overlays(const pack_version_t& version) const {
    std::vector<std::string> result;
    for (const auto& overlay : m_overlays) {
        if (overlay.m_range.contains(version)) result.push_back(overlay.m_directory);
    }
    return result;
}

}
