/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <tungsten/util/json_reflect.hpp>
#include <tungsten/resource/resource_location.hpp>
#include "feature.hpp"
#include "processor_list.hpp"

namespace tungsten {

struct template_pool_t {
    struct element_t {
        struct wrapped_template_pool_entry_t;
        struct template_pool_entry__single_pool_element_t {
            static constexpr std::string_view type_tag = "single_pool_element"; static constexpr std::string_view type_key = "element_type";
            std::optional<std::string> projection; // rigid || terrain_matching
            resource_location_t location;
            holder_t<processor_list_t> processors;
            std::optional<std::string> override_liquid_setting; // apply_waterlogging || ignore_waterlogging
        };
        struct template_pool_entry__legacy_single_pool_element_t {
            static constexpr std::string_view type_tag = "legacy_single_pool_element"; static constexpr std::string_view type_key = "element_type";
            std::optional<std::string> projection; // rigid || terrain_matching
            resource_location_t location;
            holder_t<processor_list_t> processors;
        };
        struct template_pool_entry__feature_pool_element_t {
            static constexpr std::string_view type_tag = "feature_pool_element"; static constexpr std::string_view type_key = "element_type";
            std::optional<std::string> projection; // rigid || terrain_matching
            holder_t<placed_feature_t> feature;
        };
        struct template_pool_entry__list_pool_element_t {
            static constexpr std::string_view type_tag = "list_pool_element"; static constexpr std::string_view type_key = "element_type";
            std::optional<std::string> projection; // rigid || terrain_matching
            std::vector<wrapped_template_pool_entry_t> elements;
        };
        struct template_pool_entry__empty_pool_element_t {
            static constexpr std::string_view type_tag = "empty_pool_element"; static constexpr std::string_view type_key = "element_type";
            std::optional<std::string> projection; // rigid || terrain_matching
        };

        using template_pool_entry_t = std::variant<template_pool_entry__single_pool_element_t, template_pool_entry__legacy_single_pool_element_t,
            template_pool_entry__feature_pool_element_t, template_pool_entry__list_pool_element_t, template_pool_entry__empty_pool_element_t>;
        struct wrapped_template_pool_entry_t {
            template_pool_entry_t value;

            struct glaze {
                using R = wrapped_template_pool_entry_t;
                static constexpr auto value = &R::value;
            };
        };

        template_pool_entry_t element;
        float weight;
    };

    resource_location_t fallback;
    std::vector<element_t> elements;
};

}
