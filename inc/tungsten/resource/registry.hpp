/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <tungsten/datapack/datapack_stack.hpp>
#include <tungsten/datapack/load_report.hpp>
#include <tungsten/resource/resource_location.hpp>
#include <tungsten/util/json_reflect.hpp>

namespace tungsten {

struct registry_load_options_t {
    // report keys of a document which are not part of the typed model (the wiki can lag behind the game, so this is a warning)
    bool m_warn_unknown_keys = true;
};

// how files of a registry are turned into objects; specialize for non-JSON registries
template<class T> struct registry_codec_t {
    static constexpr std::string_view extension = ".json";

    static T decode(std::span<const std::uint8_t> bytes, const resource_location_t&) {
        // like the game: comments are tolerated
        return jr::read<T>(std::string_view(reinterpret_cast<const char*>(bytes.data()), bytes.size()));
    }
};

struct generic_registry_t {
    virtual ~generic_registry_t() = default;

    // directory below data/<namespace>/ ("loot_table", "worldgen/biome")
    virtual const resource_location_t& identifier() const noexcept = 0;
    virtual std::size_t object_idx(const resource_location_t& identifier) const = 0;
    virtual std::size_t size() const = 0;
    virtual bool contains(const resource_location_t& identifier) const = 0;
    virtual std::vector<resource_location_t> ids() const = 0;
    virtual void clear() = 0;
    // name of the pack which provided the entry
    virtual std::string source_of(const resource_location_t& identifier) const = 0;
    // replaces the contents with every file found in the stack
    virtual void load(const datapack_stack_t& stack, load_report_t& report, const registry_load_options_t& options = {}) = 0;
};

template<typename T> struct registry_t : public generic_registry_t {
    mutable std::shared_mutex m_mutex;

    resource_location_t m_identifier;

    std::map<resource_location_t, std::size_t> m_entries_idx_map;
    std::map<resource_location_t, std::shared_ptr<T>> m_entries_map;
    std::vector<std::shared_ptr<T>> m_entries_linear;
    std::vector<resource_location_t> m_ids_linear;
    std::map<resource_location_t, std::string> m_sources;

    registry_t(const resource_location_t& identifier) : m_identifier(identifier) {}

    registry_t(const registry_t&) = delete;
    registry_t(registry_t&&) = delete;

    void operator=(const registry_t&) = delete;
    void operator=(registry_t&&) = delete;

    const resource_location_t& identifier() const noexcept override { return m_identifier; }

    // keeps the existing object when the identifier is already registered
    bool register_object(const resource_location_t& identifier, std::shared_ptr<T> object, std::string_view source = "") {
        std::unique_lock lock (m_mutex);
        if (m_entries_idx_map.contains(identifier)) return false;
        insert_locked(identifier, std::move(object), source);
        return true;
    }
    // inserts or replaces (the index of a replaced entry stays the same)
    void set_object(const resource_location_t& identifier, std::shared_ptr<T> object, std::string_view source = "") {
        std::unique_lock lock (m_mutex);
        auto it = m_entries_idx_map.find(identifier);
        if (it == m_entries_idx_map.end()) {
            insert_locked(identifier, std::move(object), source);
            return;
        }
        m_entries_linear[it->second] = object;
        m_entries_map[identifier] = std::move(object);
        m_sources[identifier] = std::string(source);
    }
    bool delete_object(const resource_location_t& identifier) {
        std::unique_lock lock (m_mutex);
        auto it = m_entries_idx_map.find(identifier);
        if (it == m_entries_idx_map.end()) return false;
        std::size_t idx = it->second;
        m_entries_linear.erase(m_entries_linear.begin() + static_cast<std::ptrdiff_t>(idx));
        m_ids_linear.erase(m_ids_linear.begin() + static_cast<std::ptrdiff_t>(idx));
        m_entries_map.erase(identifier);
        m_sources.erase(identifier);
        m_entries_idx_map.erase(it);
        // every entry behind the removed one shifted by one
        for (std::size_t i = idx; i < m_ids_linear.size(); ++i) m_entries_idx_map[m_ids_linear[i]] = i;
        return true;
    }
    void clear() override {
        std::unique_lock lock (m_mutex);
        m_entries_idx_map.clear();
        m_entries_map.clear();
        m_entries_linear.clear();
        m_ids_linear.clear();
        m_sources.clear();
    }

    std::size_t size() const override {
        std::shared_lock lock (m_mutex);
        return m_entries_linear.size();
    }
    bool contains(const resource_location_t& identifier) const override {
        std::shared_lock lock (m_mutex);
        return m_entries_idx_map.contains(identifier);
    }
    std::vector<resource_location_t> ids() const override {
        std::shared_lock lock (m_mutex);
        return m_ids_linear;
    }
    std::string source_of(const resource_location_t& identifier) const override {
        std::shared_lock lock (m_mutex);
        auto it = m_sources.find(identifier);
        return it == m_sources.end() ? std::string() : it->second;
    }
    std::size_t object_idx(const resource_location_t& identifier) const override {
        std::shared_lock lock (m_mutex);
        auto it = m_entries_idx_map.find(identifier);
        if (it == m_entries_idx_map.end()) {
            throw std::runtime_error("registry_t::object_idx error: unable to locate " + identifier.to_string());
        }
        return it->second;
    }

    std::shared_ptr<T> by_idx(std::size_t idx) const {
        std::shared_lock lock (m_mutex);
        if (idx >= m_entries_linear.size()) {
            throw std::runtime_error("registry_t::by_idx error: out of bounds: " + std::to_string(idx));
        }
        return m_entries_linear[idx];
    }
    std::shared_ptr<T> by_identifier(const resource_location_t& identifier) const {
        std::shared_lock lock (m_mutex);
        auto it = m_entries_map.find(identifier);
        if (it == m_entries_map.end()) {
            throw std::runtime_error("registry_t::by_identifier error: unable to locate " + identifier.to_string() + " in " + m_identifier.to_string());
        }
        return it->second;
    }
    // nullptr when absent
    std::shared_ptr<T> find(const resource_location_t& identifier) const {
        std::shared_lock lock (m_mutex);
        auto it = m_entries_map.find(identifier);
        return it == m_entries_map.end() ? nullptr : it->second;
    }

    void load(const datapack_stack_t& stack, load_report_t& report, const registry_load_options_t& options = {}) override {
        clear();
        std::map<std::string, std::set<std::string>> unknown_keys; // "<struct>::<key>" -> the files that contained it
        for (const auto& [id, layers] : stack.list_layers(m_identifier.m_path, registry_codec_t<T>::extension)) {
            const auto& winner = layers.back();
            try {
                auto bytes = winner.m_pack->read(winner.m_file);
                std::vector<std::string> unknown;
                std::optional<jr::unknown_key_scope_t> scope;
                if (options.m_warn_unknown_keys) scope.emplace(unknown);
                T object = registry_codec_t<T>::decode(bytes, id);
                scope.reset();
                for (auto& key : unknown) unknown_keys[std::move(key)].insert(id.to_string());
                set_object(id, std::make_shared<T>(std::move(object)), winner.m_pack->name());
            } catch (const jr::error& e) {
                report.error(m_identifier.m_path, id.to_string(), winner.m_pack->name(), e.what());
            } catch (const std::exception& e) {
                report.error(m_identifier.m_path, id.to_string(), winner.m_pack->name(), e.what());
            }
        }
        for (const auto& [key, files] : unknown_keys) {
            std::string list;
            for (const auto& file : files) list += (list.empty() ? "" : ", ") + file;
            report.warning(m_identifier.m_path, "", "", "unknown key (not modelled): " + key + " in " + list);
        }
    }
private:
    void insert_locked(const resource_location_t& identifier, std::shared_ptr<T> object, std::string_view source) {
        m_entries_idx_map.insert({ identifier, m_entries_linear.size() });
        m_entries_linear.push_back(object);
        m_ids_linear.push_back(identifier);
        m_entries_map.insert({ identifier, std::move(object) });
        m_sources[identifier] = std::string(source);
    }
};

}
