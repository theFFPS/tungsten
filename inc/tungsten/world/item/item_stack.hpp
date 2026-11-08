#pragma once

#include <tungsten/resource/resource_location.hpp>
#include <tungsten/world/data_component.hpp>

namespace tungsten {

struct item_stack_t {
    resource_location_t m_id;
    int m_count = 1;
    std::unordered_map<std::string, data_component_t> m_components;

    friend void to_json(nlohmann::json& j, const item_stack_t& val) {
        if (val.m_components.size()) j = {
            { "id", val.m_id },
            { "count", val.m_count },
            { "components", val.m_components }
        }; else j = {
            { "id", val.m_id },
            { "count", val.m_count }
        };
    }
    friend void from_json(const nlohmann::json& j, item_stack_t& val) {
        if (j.is_string()) {
            val.m_id = j;
            val.m_components = {};
            val.m_count = 1;
        } else {
            val.m_id = j["id"];
            if (j.contains("count")) val.m_count = j["count"];
            else val.m_count = 1;
            if (j.contains("components")) val.m_components = j["components"];
            else val.m_components = {};
        }
    }
};

}
