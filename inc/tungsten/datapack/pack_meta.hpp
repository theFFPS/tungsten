/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <compare>
#include <cstdint>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <vector>
#include <glaze/glaze.hpp>
#include <tungsten/type/text_component.hpp>

namespace tungsten {

// "<major>.<minor>" pack version. since 1.21.9 (data pack format 88.0) formats carry a minor part
struct pack_version_t {
    std::int32_t m_major = 0;
    std::int32_t m_minor = 0;

    static constexpr std::int32_t any_minor = 0x7fffffff;

    auto operator<=>(const pack_version_t&) const = default;
    std::string to_string() const;

    // an integer N means N.0, [N] means N.0, [N, M] means N.M
    static pack_version_t parse_min(const glz::generic& j);
    // an integer N means N.<any minor>, [N] means N.<any minor>, [N, M] means N.M
    static pack_version_t parse_max(const glz::generic& j);
};

// the pack version of the game this parser targets (see version.json: pack_version.data_major / data_minor)
inline constexpr pack_version_t current_data_pack_version { 121, 0 };
// since this format min_format / max_format replace pack_format / supported_formats
inline constexpr std::int32_t pack_format_with_min_max = 82;

struct pack_version_range_t {
    pack_version_t m_min;
    pack_version_t m_max;

    bool contains(const pack_version_t& version) const noexcept { return m_min <= version && version <= m_max; }
};

struct pack_filter_pattern_t {
    std::optional<std::string> m_namespace; // regular expression, matches every namespace when absent
    std::optional<std::string> m_path;      // regular expression, matches every path when absent
    std::regex m_namespace_regex;
    std::regex m_path_regex;

    // path is relative to the namespace directory and includes the extension ("loot_table/blocks/stone.json")
    bool matches(std::string_view namespace_s, std::string_view path) const;
};

struct pack_overlay_t {
    std::string m_directory;
    pack_version_range_t m_range;
};

struct pack_meta_t {
    glz::generic m_description;
    std::optional<std::int32_t> m_pack_format; // legacy
    pack_version_range_t m_range;
    std::vector<resource_location_t> m_enabled_features;
    std::vector<pack_filter_pattern_t> m_filter_block;
    std::vector<pack_overlay_t> m_overlays;
    glz::generic m_raw;

    // throws std::runtime_error with a descriptive message when pack.mcmeta is malformed
    static pack_meta_t parse(std::string_view text);
    static pack_meta_t from_json(const glz::generic& j);

    // parses the description as a text component (falls back to a plain string on failure)
    text_t description_text() const;
    bool is_filtered(std::string_view namespace_s, std::string_view path) const;
    // overlay directories which apply to the given version, in application order
    std::vector<std::string> active_overlays(const pack_version_t& version) const;
};

}
