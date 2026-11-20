/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

struct processor_list_t {
    struct processor_t {
        // a processor is a typed object; its processor_type selects the processor
        // kept raw until the processor types are modelled
        glz::generic raw;

        void from_generic(const glz::generic& j) {
            if (!j.is_object() || !j.contains("processor_type")) {
                throw jr::error("", "entries require a 'processor_type'");
            }
            raw = j;
        }
        glz::generic to_generic() const { return raw; }

        struct glaze {
            using R = processor_t;
            static constexpr auto value = glz::custom<&R::from_generic, &R::to_generic>;
        };
    };

    std::vector<processor_t> processors;
};

}
