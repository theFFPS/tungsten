#pragma once

#include <optional>
#include <array>
#include <string>
#include <vector>

namespace tungsten {

struct game_profile_property_t {
    std::string name;
    std::string value;
    std::optional<std::string> signature;
};
struct game_profile_t {
    std::optional<std::string> name;
    std::optional<std::array<int, 4>> id;
    std::optional<std::vector<game_profile_property_t>> properties;
    std::string texture = "minecraft:entity/player/slim/alex";
    std::optional<std::string> cape;
    std::string elytra = "minecraft:entity/equipment/wings/elytra";
    std::string model = "slim";
};

}
