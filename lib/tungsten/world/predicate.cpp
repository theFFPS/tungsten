/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/world/predicate.hpp>
#include <tungsten/world/block/block_entity.hpp>
#include <tungsten/world/context_number_provider.hpp>

namespace tungsten {

void wrapped_wrapped_context_number_provider_t::from_generic(const glz::generic& j) {
    value = std::make_shared<wrapped_context_number_provider_t>(jr::from_generic<wrapped_context_number_provider_t>(j));
}
glz::generic wrapped_wrapped_context_number_provider_t::to_generic() const {
    return jr::to_generic(*value);
}

void wrapped_item_predicate_t::from_generic(const glz::generic& j) {
    value = std::make_shared<item_predicate_t>(jr::from_generic<item_predicate_t>(j));
}
glz::generic wrapped_item_predicate_t::to_generic() const {
    return jr::to_generic(*value);
}

void data_component_predicate_t::from_generic(const glz::generic& j) {
    using jr::from_generic;
    auto is = [this](const char* id) { return type == resource_location_t(id); };
    auto sub = [&j](const char* key, auto& out) { jr::read_key(j, key, out); };

    if (is("attribute_modifiers") && j.is_object()) {
        attribute_modifiers_t v; sub("modifiers", v.modifiers); value = std::move(v);
    } else if (is("bundle_contents") && j.is_object()) {
        bundle_contents_t v; sub("items", v.items); value = std::move(v);
    } else if (is("container") && j.is_object()) {
        container_t v; sub("items", v.items); value = std::move(v);
    } else if ((is("enchantments") || is("stored_enchantments")) && j.is_array()) {
        value = from_generic<std::vector<enchantment_t>>(j);
    } else if (is("firework_explosion") && j.is_object()) {
        value = from_generic<firework_explosion_t>(j);
    } else if (is("writable_book_content") && j.is_object()) {
        writable_book_content_t v; sub("pages", v.pages); value = std::move(v);
    } else if (is("jukebox_playable") && j.is_object()) {
        jukebox_playable_t v; sub("song", v.song); value = std::move(v);
    } else if (is("custom_data")) {
        if (j.is_string()) value = custom_data_t(j.get_string());
        else value = custom_data_t(j);
    } else if (is("damage") && j.is_object()) {
        damage_t v; sub("damage", v.damage); sub("durability", v.durability); value = std::move(v);
    } else if (is("trim") && j.is_object()) {
        trim_t v; sub("material", v.material); sub("pattern", v.pattern); value = std::move(v);
    } else if (is("potion_contents") && j.is_object()) {
        potion_contents_t v; sub("potions", v.potions); sub("effects", v.effects); value = std::move(v);
    } else if (is("potion_contents") && (j.is_string() || j.is_array())) {
        potion_contents_t v; v.potions = from_generic<holder_set_t>(j); value = std::move(v);
    } else if (is("fireworks") && j.is_object()) {
        fireworks_t v; sub("explosions", v.explosions); sub("flight_duration", v.flight_duration); value = std::move(v);
    } else if (is("written_book_content") && j.is_object()) {
        written_book_content_t v;
        sub("pages", v.pages); sub("author", v.author); sub("title", v.title); sub("resolved", v.resolved); sub("generation", v.generation);
        value = std::move(v);
    }
}

glz::generic data_component_predicate_t::to_generic() const {
    glz::generic j = glz::generic::object_t{};
    std::visit(overloaded{
        [&j](const attribute_modifiers_t& v) { jr::write_key(j, "modifiers", v.modifiers); },
        [&j](const bundle_contents_t& v) { jr::write_key(j, "items", v.items); },
        [&j](const container_t& v) { jr::write_key(j, "items", v.items); },
        [&j](const custom_data_t& v) {
            std::visit(overloaded{
                [&j](const std::string& text) { j = glz::generic(text); },
                [&j](const glz::generic& any) { j = any; }
            }, v);
        },
        [&j](const damage_t& v) { jr::write_key(j, "damage", v.damage); jr::write_key(j, "durability", v.durability); },
        [&j](const firework_explosion_t& v) { j = jr::to_generic(v); },
        [&j](const fireworks_t& v) { jr::write_key(j, "explosions", v.explosions); jr::write_key(j, "flight_duration", v.flight_duration); },
        [&j](const jukebox_playable_t& v) { jr::write_key(j, "song", v.song); },
        [&j](const potion_contents_t& v) { jr::write_key(j, "potions", v.potions); jr::write_key(j, "effects", v.effects); },
        [&j](const std::vector<enchantment_t>& v) { j = jr::to_generic(v); },
        [&j](const trim_t& v) { jr::write_key(j, "material", v.material); jr::write_key(j, "pattern", v.pattern); },
        [&j](const writable_book_content_t& v) { jr::write_key(j, "pages", v.pages); },
        [&j](const written_book_content_t& v) {
            jr::write_key(j, "pages", v.pages);
            jr::write_key(j, "author", v.author);
            jr::write_key(j, "title", v.title);
            jr::write_key(j, "generation", v.generation);
            jr::write_key(j, "resolved", v.resolved);
        }
    }, value);
    return j;
}

void data_component_predicates_t::from_generic(const glz::generic& j) {
    *this = data_component_predicates_t();
    if (!j.is_object()) throw jr::error("", "expected an object");
    for (const auto& [key, element] : j.get_object()) {
        try {
            if (key.starts_with('!')) removed.emplace_back(std::string_view(key).substr(1));
            else {
                data_component_predicate_t predicate;
                predicate.type.from(key);
                predicate.from_generic(element);
                value.emplace(resource_location_t(key), std::move(predicate));
            }
        } catch (const std::exception& e) {
            throw jr::error("[" + key + "]", e.what());
        }
    }
}
glz::generic data_component_predicates_t::to_generic() const {
    glz::generic j = glz::generic::object_t{};
    for (const auto& [id, predicate] : value) j[id.to_string()] = predicate.to_generic();
    for (const auto& id : removed) j["!" + id.to_string()] = glz::generic::object_t{};
    return j;
}

}
