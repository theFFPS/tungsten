#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace tungsten {

struct damage_type_t {
    std::string m_message_id;
    std::string m_scaling; // never || always || when_caused_by_living_non_player
    std::string m_effects = "hurt"; // hurt || thorns || drowning || burning || poking || freezing
    std::string m_death_message_type = "default"; // default || fall_variants || intentional_game_design
    float m_exhaustion;

    friend void to_json(nlohmann::json& j, const damage_type_t& val) {
        j = {
            { "message_id", val.m_message_id },
            { "exhaustion", val.m_exhaustion },
            { "scaling", val.m_scaling },
            { "effects", val.m_effects },
            { "death_message_type", val.m_death_message_type }
        };
    }
    friend void from_json(const nlohmann::json& j, damage_type_t& val) {
        val.m_message_id = j["message_id"];
        val.m_scaling = j["scaling"];
        val.m_exhaustion = j["exhaustion"];
        if (j.contains("effects")) val.m_effects = std::string(j["effects"]);
        else val.m_effects = "hurt";
        if (j.contains("death_message_type")) val.m_death_message_type = std::string(j["death_message_type"]);
        else val.m_death_message_type = "hurt";
    }
};

}
