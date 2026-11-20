/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/nbt/nbt.hpp>
#include <bit>
#include <cstring>
#include <stdexcept>

import tungsten.compression;

namespace tungsten {

namespace {

constexpr int max_depth = 512;

struct reader_t {
    std::span<const std::uint8_t> m_data;
    std::size_t m_pos = 0;

    [[noreturn]] void fail(const std::string& message) const {
        throw std::runtime_error("nbt_t::parse error: " + message + " (at offset " + std::to_string(m_pos) + ")");
    }
    void need(std::size_t count) const {
        if (m_data.size() - m_pos < count) fail("unexpected end of data");
    }
    template<class T> T read_int() {
        need(sizeof(T));
        std::make_unsigned_t<T> raw = 0;
        for (std::size_t i = 0; i < sizeof(T); ++i) raw = static_cast<std::make_unsigned_t<T>>((raw << 8) | m_data[m_pos + i]);
        m_pos += sizeof(T);
        return static_cast<T>(raw);
    }
    float read_float() {
        auto bits = static_cast<std::uint32_t>(read_int<std::int32_t>());
        return std::bit_cast<float>(bits);
    }
    double read_double() {
        auto bits = static_cast<std::uint64_t>(read_int<std::int64_t>());
        return std::bit_cast<double>(bits);
    }
    std::size_t read_length(std::size_t element_size) {
        std::int32_t length = read_int<std::int32_t>();
        if (length < 0) fail("negative array length");
        // refuse lengths which can not possibly fit into the remaining data
        if (element_size != 0 && static_cast<std::size_t>(length) > (m_data.size() - m_pos) / element_size) fail("array length exceeds the data");
        return static_cast<std::size_t>(length);
    }
    std::string read_string() {
        auto length = static_cast<std::uint16_t>(read_int<std::int16_t>());
        need(length);
        std::string result(reinterpret_cast<const char*>(m_data.data() + m_pos), length);
        m_pos += length;
        return result;
    }

    nbt_t read_payload(nbt_type_t type, int depth) {
        if (depth > max_depth) fail("nesting is too deep");
        nbt_t result;
        switch (type) {
        case nbt_type_t::byte: result.m_value = read_int<std::int8_t>(); break;
        case nbt_type_t::short_: result.m_value = read_int<std::int16_t>(); break;
        case nbt_type_t::int_: result.m_value = read_int<std::int32_t>(); break;
        case nbt_type_t::long_: result.m_value = read_int<std::int64_t>(); break;
        case nbt_type_t::float_: result.m_value = read_float(); break;
        case nbt_type_t::double_: result.m_value = read_double(); break;
        case nbt_type_t::byte_array: {
            std::size_t length = read_length(1);
            std::vector<std::int8_t> values(length);
            std::memcpy(values.data(), m_data.data() + m_pos, length);
            m_pos += length;
            result.m_value = std::move(values);
            break;
        }
        case nbt_type_t::string: result.m_value = read_string(); break;
        case nbt_type_t::list: {
            nbt_list_t list;
            auto element_type = static_cast<nbt_type_t>(read_int<std::int8_t>());
            list.m_element_type = element_type;
            std::int32_t length = read_int<std::int32_t>();
            if (length < 0) length = 0;
            if (element_type == nbt_type_t::end && length > 0) fail("a non-empty list of TAG_End");
            if (static_cast<std::uint8_t>(element_type) > 12) fail("unknown list element type " + std::to_string(static_cast<int>(element_type)));
            // every element takes at least one byte except for empty compounds (1 byte for TAG_End), so this bound is safe
            if (static_cast<std::size_t>(length) > m_data.size() - m_pos) fail("list length exceeds the data");
            list.m_items.reserve(static_cast<std::size_t>(length));
            for (std::int32_t i = 0; i < length; ++i) list.m_items.push_back(read_payload(element_type, depth + 1));
            result.m_value = std::move(list);
            break;
        }
        case nbt_type_t::compound: {
            nbt_compound_t compound;
            while (true) {
                auto child_type = static_cast<nbt_type_t>(read_int<std::int8_t>());
                if (child_type == nbt_type_t::end) break;
                if (static_cast<std::uint8_t>(child_type) > 12) fail("unknown tag type " + std::to_string(static_cast<int>(child_type)));
                std::string name = read_string();
                compound[std::move(name)] = read_payload(child_type, depth + 1);
            }
            result.m_value = std::move(compound);
            break;
        }
        case nbt_type_t::int_array: {
            std::size_t length = read_length(4);
            std::vector<std::int32_t> values(length);
            for (auto& v : values) v = read_int<std::int32_t>();
            result.m_value = std::move(values);
            break;
        }
        case nbt_type_t::long_array: {
            std::size_t length = read_length(8);
            std::vector<std::int64_t> values(length);
            for (auto& v : values) v = read_int<std::int64_t>();
            result.m_value = std::move(values);
            break;
        }
        default: fail("unexpected tag type " + std::to_string(static_cast<int>(type)));
        }
        return result;
    }
};

}

nbt_type_t nbt_t::type() const noexcept {
    switch (m_value.index()) {
    case 0: return nbt_type_t::byte;
    case 1: return nbt_type_t::short_;
    case 2: return nbt_type_t::int_;
    case 3: return nbt_type_t::long_;
    case 4: return nbt_type_t::float_;
    case 5: return nbt_type_t::double_;
    case 6: return nbt_type_t::byte_array;
    case 7: return nbt_type_t::string;
    case 8: return nbt_type_t::list;
    case 9: return nbt_type_t::compound;
    case 10: return nbt_type_t::int_array;
    default: return nbt_type_t::long_array;
    }
}
const nbt_compound_t& nbt_t::compound() const {
    if (const auto* v = std::get_if<nbt_compound_t>(&m_value)) return *v;
    throw std::runtime_error("nbt_t::compound error: tag is not a compound");
}
const nbt_list_t& nbt_t::list() const {
    if (const auto* v = std::get_if<nbt_list_t>(&m_value)) return *v;
    throw std::runtime_error("nbt_t::list error: tag is not a list");
}
const std::string& nbt_t::string() const {
    if (const auto* v = std::get_if<std::string>(&m_value)) return *v;
    throw std::runtime_error("nbt_t::string error: tag is not a string");
}
const nbt_t* nbt_t::find(std::string_view key) const noexcept {
    const auto* compound = std::get_if<nbt_compound_t>(&m_value);
    if (compound == nullptr) return nullptr;
    auto it = compound->find(std::string(key));
    return it == compound->end() ? nullptr : &it->second;
}
std::int64_t nbt_t::as_integer() const {
    switch (m_value.index()) {
    case 0: return std::get<std::int8_t>(m_value);
    case 1: return std::get<std::int16_t>(m_value);
    case 2: return std::get<std::int32_t>(m_value);
    case 3: return std::get<std::int64_t>(m_value);
    default: throw std::runtime_error("nbt_t::as_integer error: tag is not an integer");
    }
}
double nbt_t::as_floating() const {
    if (const auto* f = std::get_if<float>(&m_value)) return *f;
    if (const auto* d = std::get_if<double>(&m_value)) return *d;
    return static_cast<double>(as_integer());
}

nbt_t nbt_t::parse(std::span<const std::uint8_t> bytes, std::string* root_name) {
    std::vector<std::uint8_t> decompressed;
    // gzip magic 1f 8b, or a zlib header (0x78 and a valid check value)
    bool gzip = bytes.size() >= 2 && bytes[0] == 0x1f && bytes[1] == 0x8b;
    bool zlib = bytes.size() >= 2 && (bytes[0] & 0x0f) == 8 && ((bytes[0] << 8) | bytes[1]) % 31 == 0 && bytes[0] != 0x0a;
    if (gzip || zlib) {
        decompressed = backend::zlib_compression_t::gzip_decompress(bytes.data(), bytes.size(), bytes.size() * 8);
        bytes = decompressed;
    }
    reader_t reader { bytes };
    auto type = static_cast<nbt_type_t>(reader.read_int<std::int8_t>());
    if (type != nbt_type_t::compound && type != nbt_type_t::list) reader.fail("the root tag must be a compound or a list");
    std::string name = reader.read_string();
    if (root_name != nullptr) *root_name = std::move(name);
    return reader.read_payload(type, 0);
}

glz::generic nbt_t::to_json() const {
    return std::visit([](const auto& v) -> glz::generic {
        using V = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<V, nbt_list_t>) {
            glz::generic::array_t array;
            for (const auto& item : v.m_items) array.push_back(item.to_json());
            return glz::generic(std::move(array));
        } else if constexpr (std::is_same_v<V, nbt_compound_t>) {
            glz::generic object = glz::generic::object_t{};
            for (const auto& [key, item] : v) object[key] = item.to_json();
            return object;
        } else if constexpr (std::is_arithmetic_v<V>) {
            return glz::generic(static_cast<double>(v));
        } else if constexpr (std::is_same_v<V, std::string>) {
            return glz::generic(v);
        } else {
            // arrays of numbers
            glz::generic::array_t array;
            for (const auto& element : v) array.emplace_back(static_cast<double>(element));
            return glz::generic(std::move(array));
        }
    }, m_value);
}

}
