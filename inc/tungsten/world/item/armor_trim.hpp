#pragma once

#include <tungsten/resource/resource_location.hpp>
#include <tungsten/type/text_component.hpp>

namespace tungsten {

struct trim_material_t {
    resource_location_t palette_id;
    text_t description;
};

struct trim_pattern_t {
    resource_location_t m_asset_id;
    bool m_decal = false;
    text_t m_description;

    friend void to_json(nlohmann::json& j, const trim_pattern_t& val) {
        j = {
            { "asset_id", val.m_asset_id },
            { "decal", val.m_decal },
            { "description", val.m_description }
        };
    }
    friend void from_json(const nlohmann::json& j, trim_pattern_t& val) {
        val.m_asset_id = j["asset_id"];
        val.m_description = j["description"];
        if (j.contains("decal")) val.m_decal = j["decal"];
        else val.m_decal = false;
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(trim_material_t, palette_id, description)

}
