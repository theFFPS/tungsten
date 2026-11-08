#pragma once

#include <tungsten/resource/resource_location.hpp>

namespace tungsten {

struct decorated_pot_pattern_t {
    resource_location_t asset_id;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(decorated_pot_pattern_t, asset_id)

}
