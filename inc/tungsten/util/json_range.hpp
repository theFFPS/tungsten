/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <optional>
#include <vector>
#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

// a number (min == max), [min, max] or { "min": .., "max": .. }
template<class T> struct range_t {
    std::optional<T> min;
    std::optional<T> max;

    void from_generic(const glz::generic& j) {
        *this = range_t();
        if (is_scalar(j)) {
            min = max = scalar(j);
        } else if (j.is_object()) {
            if (j.contains("min")) min = scalar(j["min"]);
            if (j.contains("max")) max = scalar(j["max"]);
        } else if (j.is_array() && j.get_array().size() == 2) {
            min = scalar(j[0]);
            max = scalar(j[1]);
        } else {
            throw jr::error("", "expected a scalar, array of size 2 or a range object");
        }
    }
    glz::generic to_generic() const {
        if (min.has_value() && max.has_value() && *min == *max) return from_scalar(*min);
        glz::generic j = glz::generic::object_t{};
        if (min.has_value()) j["min"] = from_scalar(*min);
        if (max.has_value()) j["max"] = from_scalar(*max);
        return j;
    }

private:
    static bool is_scalar(const glz::generic& j) {
        if constexpr (std::is_arithmetic_v<T>) return j.is_number();
        else return !j.is_object() && !j.is_array() && !j.is_null();
    }
    static T scalar(const glz::generic& j) {
        if constexpr (std::is_arithmetic_v<T>) {
            if (!j.is_number()) throw jr::error("", "expected a number");
            return static_cast<T>(j.get_number());
        } else {
            return jr::from_generic<T>(j);
        }
    }
    static glz::generic from_scalar(const T& v) {
        if constexpr (std::is_arithmetic_v<T>) return glz::generic(static_cast<double>(v));
        else return jr::to_generic(v);
    }

public:
    struct glaze {
        using R = range_t;
        static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
    };
};

using int_range_t = range_t<int>;
using float_range_t = range_t<float>;
using double_range_t = range_t<double>;

template<class T> struct count_entry_t {
    std::optional<T> test;
    std::optional<int_range_t> count;
};
template<class T> struct collection_predicate_t {
    std::vector<T> contains = {};
    std::optional<int_range_t> size;
    std::vector<count_entry_t<T>> count = {};
};

}
