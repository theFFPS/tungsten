/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

// glaze-backed JSON support. Reflection-based (de)serialization is provided by glaze itself via
// C++26 reflection; this header keeps the shared diagnostics and the namespaced-tag support.
//
//  - jr::error               : exception carrying a JSON path + message (thrown by hand-written readers)
//  - jr::unknown_key_scope_t : collects "<struct>::<key>" entries for unmodelled keys while alive
//  - jr::normalize_tag       : strips a leading "minecraft:" so namespaced and bare variant tags match
//  - jr::read_opts / jr::read<T> : glaze options + helper used for datapack JSON (unknown keys are ignored)
//
// Conventions (replace TUNGSTEN_JSON / TUNGSTEN_JSON_TAGGED):
//  - plain structs are just aggregates; glaze reflects them (C++26 reflection). Members carry no m_ prefix, so
//    the member name is the JSON key. Keys that are not valid identifiers use glz::meta explicitly.
//  - a tagged alternative declares `static constexpr std::string_view type_tag = "constant";`
//    (optionally `type_key = "id"`; the default discriminator key is "type").
//  - std::variant<Ts...> whose alternatives (shared_ptr/unique_ptr unwrapped) ALL declare type_tag is read/written
//    as an internally tagged object {"type": "...", ...fields}; both "constant" and "minecraft:constant" match.
//  - every other std::variant is an untagged union: the first alternative that parses wins (old semantics).

#include <array>
#include <memory>
#include <optional>
#include <stdexcept>
#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <glaze/glaze.hpp>

namespace tungsten::jr {

class error final : public std::runtime_error {
public:
    std::string m_path;
    std::string m_message;

    error(std::string path, std::string message) : std::runtime_error(path.empty() ? message : path + ": " + message),
        m_path(std::move(path)), m_message(std::move(message)) {}
};

// collects unknown keys while alive on the current thread; entries are "<struct>::<key>"
struct unknown_key_scope_t {
    std::vector<std::string>* m_previous;
    explicit unknown_key_scope_t(std::vector<std::string>& sink);
    ~unknown_key_scope_t();
    unknown_key_scope_t(const unknown_key_scope_t&) = delete;
    void operator=(const unknown_key_scope_t&) = delete;
};
std::vector<std::string>*& unknown_key_sink() noexcept;

// Variant discriminators may be written bare ("constant") or namespaced ("minecraft:constant").
// Both denote the same alternative; this returns the bare form used for glaze's exact-match lookup.
inline std::string_view normalize_tag(std::string_view tag) noexcept {
    constexpr std::string_view ns = "minecraft:";
    if (tag.starts_with(ns)) tag.remove_prefix(ns.size());
    return tag;
}

}

namespace tungsten::jr {

// datapack JSON ignores keys that are not modelled
inline constexpr glz::opts read_opts{.comments = true, .error_on_unknown_keys = false, .error_on_missing_keys = false}; // like the game, comments are tolerated

template<class T> struct unwrap_pointer { using type = T; };
template<class T> struct unwrap_pointer<std::shared_ptr<T>> { using type = T; };
template<class T, class D> struct unwrap_pointer<std::unique_ptr<T, D>> { using type = T; };
template<class T> using unwrap_pointer_t = typename unwrap_pointer<T>::type;

template<class T> concept has_type_tag = requires { { unwrap_pointer_t<T>::type_tag } -> std::convertible_to<std::string_view>; };

template<class T> inline constexpr std::string_view type_key_of_v = [] {
    if constexpr (requires { { unwrap_pointer_t<T>::type_key } -> std::convertible_to<std::string_view>; }) return std::string_view(unwrap_pointer_t<T>::type_key);
    else return std::string_view("type");
}();

template<class T> constexpr std::string_view type_name_of() { return glz::name_v<T>; }

template<class... Ts> inline constexpr bool all_tagged_v = (has_type_tag<Ts> && ...);

template<class V> struct tagged_variant_info;
template<class... Ts> struct tagged_variant_info<std::variant<Ts...>> {
    static constexpr std::string_view key = type_key_of_v<std::variant_alternative_t<0, std::variant<Ts...>>>;
    static constexpr std::array<std::string_view, sizeof...(Ts)> ids = {std::string_view(unwrap_pointer_t<Ts>::type_tag)...};
    static_assert(((type_key_of_v<Ts> == key) && ...), "all alternatives of a tagged variant must share one type_key");
    static constexpr bool unique = [] {
        for (size_t i = 0; i < ids.size(); ++i) for (size_t j = i + 1; j < ids.size(); ++j) if (ids[i] == ids[j]) return false;
        return true;
    }();
    static_assert(unique, "type_tag values of a tagged variant must be unique");
};

// Marker base: reads an untagged union alternative by alternative (first one that parses wins).
struct mixed_meta { static constexpr bool custom_read = true; };

inline constexpr glz::opts strict_opts{.comments = true, .error_on_unknown_keys = true, .error_on_missing_keys = false};

// the key a glaze unknown_key error points into (the error offset is inside the key string)
// inline std::string unknown_key_at(std::string_view json, size_t offset) {
//     if (offset > json.size()) return "?";
//     auto end = json.find('"', offset);
//     return std::string(json.substr(offset, end == std::string_view::npos ? std::string_view::npos : end - offset));
// }

inline std::string unknown_key_at(std::string_view json, size_t offset) {
    if (offset > json.size()) return "?";
    // Find the ':' that terminates this key-value pair (or start at offset if already on ':')
    auto colon = json.find(':', offset > 0 ? offset - 1 : 0);
    if (colon == std::string_view::npos) return "?";
    // The closing quote of the key is immediately before the colon (ignoring whitespace)
    auto close_quote = json.rfind('"', colon);
    if (close_quote == std::string_view::npos || close_quote == 0) return "?";
    auto open_quote = json.rfind('"', close_quote - 1);
    if (open_quote == std::string_view::npos) return "?";
    return std::string(json.substr(open_quote + 1, close_quote - open_quote - 1));
}

// Parses a whole document; throws jr::error (with the glaze message) on failure.
//  - while a unknown_key_scope_t is alive the first unmodelled key of the document is reported into it
//    (glaze stops at the first one, so this is one key per document, not one per struct)
//  - T::post_load() runs after a successful parse
template<class T> T read(std::string_view json) {
    T value{};
    glz::error_ctx ec{};
    if (auto* sink = unknown_key_sink()) {
        ec = glz::read<strict_opts>(value, json);
        if (ec) {
            // an empty key is not an unmodelled member (e.g. a top-level array), so it is not reported
            if (ec.ec == glz::error_code::unknown_key) {
                const std::string key = unknown_key_at(json, ec.count);
                if (key.find_first_not_of(" \t\r\n\"") != std::string::npos) sink->push_back(std::string(type_name_of<T>()) + "::" + key);
            }
            value = T{};
            ec = glz::read<read_opts>(value, json);
        }
    } else {
        ec = glz::read<read_opts>(value, json);
    }
    if (ec) throw error("", glz::format_error(ec, json));
    if constexpr (requires { value.post_load(); }) value.post_load();
    return value;
}

}

namespace tungsten::jr {

// Bridges between a document tree and a reflected type (used by hand written readers).
// Scalars take a fast path; everything else goes through glaze.
template<class T> T from_generic(const glz::generic& j) {
    if constexpr (std::is_same_v<T, bool>) {
        if (!j.is_boolean()) throw error("", "expected a boolean");
        return j.get_boolean();
    } else if constexpr (std::is_arithmetic_v<T>) {
        if (!j.is_number()) throw error("", "expected a number");
        return static_cast<T>(j.get_number());
    } else if constexpr (std::is_same_v<T, std::string>) {
        if (!j.is_string()) throw error("", "expected a string");
        return j.get_string();
    } else {
        std::string text;
        if (auto ec = glz::write_json(j, text)) throw error("", glz::format_error(ec));
        T value{};
        if (auto ec = glz::read<read_opts>(value, text)) throw error("", glz::format_error(ec, text));
        if constexpr (requires { value.post_load(); }) value.post_load();
        return value;
    }
}
template<class T> glz::generic to_generic(const T& value) {
    if constexpr (std::is_same_v<T, bool>) {
        return glz::generic(value);
    } else if constexpr (std::is_arithmetic_v<T>) {
        return glz::generic(static_cast<double>(value));
    } else if constexpr (std::is_same_v<T, std::string>) {
        return glz::generic(value);
    } else {
        std::string text;
        if (auto ec = glz::write_json(value, text)) throw error("", glz::format_error(ec));
        glz::generic j;
        if (auto ec = glz::read_json(j, text)) throw error("", glz::format_error(ec, text));
        return j;
    }
}

// "minecraft:foo" and "foo" are the same key of an object
inline const glz::generic* find_key(const glz::generic& j, const std::string& name) {
    if (!j.is_object()) return nullptr;
    if (j.contains(name)) return &j[name];
    const std::string namespaced = "minecraft:" + name;
    if (j.contains(namespaced)) return &j[namespaced];
    return nullptr;
}
// reads `out` from the key when it is present (the error path is prefixed with the key)
template<class T> void read_key(const glz::generic& j, const std::string& name, T& out) {
    const glz::generic* v = find_key(j, name);
    if (!v) return;
    try {
        out = from_generic<T>(*v);
    } catch (const error& e) {
        throw error(name + (e.m_path.empty() ? "" : (e.m_path.starts_with('[') ? "" : ".") + e.m_path), e.m_message);
    }
}
template<class T> void write_key(glz::generic& j, const std::string& name, const std::optional<T>& value) {
    if (value.has_value()) j[name] = to_generic(*value);
}
template<class T> void write_key(glz::generic& j, const std::string& name, const T& value) {
    j[name] = to_generic(value);
}

}

// ---- glaze integration ----
template <class... Ts> requires(sizeof...(Ts) > 0 && tungsten::jr::all_tagged_v<Ts...>)
struct glz::meta<std::variant<Ts...>> {
    using info = tungsten::jr::tagged_variant_info<std::variant<Ts...>>;
    static constexpr std::string_view tag = info::key;
    static constexpr auto ids = info::ids;
};

template <class... Ts> requires(sizeof...(Ts) > 0 && !tungsten::jr::all_tagged_v<Ts...>)
struct glz::meta<std::variant<Ts...>> : tungsten::jr::mixed_meta {};

namespace glz {

// "minecraft:foo" and "foo" denote the same discriminator. Glaze's stock lookup is an exact match, so strip
// the default namespace first. More constrained than the stock specialization, so it is preferred.
template <is_variant T> requires(tagged<T> && std::same_as<std::decay_t<decltype(ids_v<T>[0])>, std::string_view>)
struct variant_id_to_index<T, false> {
    using keys_t = keys_wrapper<ids_v<T>>;
    static constexpr auto& HashInfo = hash_info<keys_t>;
    static constexpr auto N = ids_v<T>.size();

    GLZ_ALWAYS_INLINE static constexpr size_t op(const char* data, const char* end, size_t n) noexcept {
        constexpr std::string_view ns = "minecraft:";
        if (n > ns.size() && std::string_view(data, ns.size()) == ns) { data += ns.size(); n -= ns.size(); }
        const auto index = decode_hash_with_size<JSON, keys_t, HashInfo, HashInfo.type>::op(data, end, n);
        if (index < N && ids_v<T>[index] == std::string_view{data, n}) return index;
        return N;
    }
};

// untagged union: first alternative that parses wins
template <is_variant T> requires(requires { requires glz::meta<T>::custom_read; } && not tagged<T>)
struct from<JSON, T> {
    template <auto Opts>
    static void op(auto&& value, is_context auto&& ctx, auto&& it, auto&& end) {
        if (skip_ws<Opts>(ctx, it, end)) return;
        auto* start = it;
        skip_value<JSON>::template op<Opts>(ctx, it, end);
        if (ctx.error == error_code::unexpected_end && it == end && it != start) ctx.error = error_code::none; // top level scalar
        if (bool(ctx.error)) return;
        const std::string_view span{start, size_t(it - start)};
        constexpr auto sub = [] { auto o = Opts; o.internal = 0; o.null_terminated = false; return o; }();
        bool done = false;
        for_each<std::variant_size_v<std::decay_t<T>>>([&]<auto I>() {
            if (done) return;
            std::variant_alternative_t<I, std::decay_t<T>> alt{};
            // a custom reader may throw jr::error to reject its shape; that means "not this alternative", so try the next
            try {
                if (!glz::read<sub>(alt, span)) { value = std::move(alt); done = true; }
            } catch (const tungsten::jr::error&) {
            }
        });
        if (!done) ctx.error = error_code::no_matching_variant_type;
    }
};

}
