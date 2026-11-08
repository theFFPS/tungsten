#pragma once

#include <nlohmann/json.hpp>

namespace tungsten {

struct item_stack_t;
struct data_component_t {
    std::string placeholder;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(data_component_t, placeholder)

}
