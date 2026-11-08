/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include "resource_location.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <vector>
#include <nlohmann/json.hpp>

namespace tungsten {

struct generic_registry_t {
    virtual ~generic_registry_t() = default;

    virtual size_t object_idx(const resource_location_t& identifier) const = 0;
};
template<typename T> struct registry_t : public generic_registry_t {
    mutable std::shared_mutex m_mutex;

    resource_location_t m_identifier;

    std::map<resource_location_t, size_t> m_entries_idx_map;
    std::map<resource_location_t, std::shared_ptr<T>> m_entries_map;
    std::vector<std::shared_ptr<T>> m_entries_linear;

    registry_t(const resource_location_t& identifier) : m_identifier(identifier) {}

    registry_t(const registry_t&) = delete;
    registry_t(registry_t&&) = delete;

    void operator=(const registry_t&) = delete;
    void operator=(registry_t&&) = delete;

    void register_object(const resource_location_t& identifier, std::shared_ptr<T> object) noexcept {
        {
            std::shared_lock lock (m_mutex);
            if (m_entries_idx_map.contains(identifier)) return;
        }
        std::unique_lock lock (m_mutex);
        m_entries_idx_map.insert({ identifier, m_entries_linear.size() });
        m_entries_linear.push_back(object);
        m_entries_map.insert({ identifier, object });
    }
    void delete_object(const resource_location_t& identifier) noexcept {
        {
            std::shared_lock lock (m_mutex);
            if (!m_entries_idx_map.contains(identifier)) return;
        }
        std::unique_lock lock (m_mutex);
        size_t idx = m_entries_idx_map[identifier];
        m_entries_idx_map.erase(identifier);
        m_entries_map.erase(identifier);
        m_entries_linear.erase(m_entries_linear.begin() + idx);
    }
    size_t object_idx(const resource_location_t& identifier) const override {
        std::shared_lock lock (m_mutex);
        if (!m_entries_idx_map.contains(identifier)) {
            throw std::runtime_error("registry_t::object_idx error: unable to locate " + identifier.to_string());
        }
        return m_entries_idx_map.at(identifier);
    }

    std::shared_ptr<T> by_idx(size_t idx) const {
        std::shared_lock lock (m_mutex);
        if (idx >= m_entries_linear.size()) {
            throw std::runtime_error("registry_t::by_idx error: out of bounds: " + std::to_string(idx));
        }
        return m_entries_linear[idx];
    }
    std::shared_ptr<T> by_identifier(const resource_location_t& identifier) const {
        std::shared_lock lock (m_mutex);
        if (!m_entries_map.contains(identifier)) {
            throw std::runtime_error("registry_t::by_identifier error: unable to locate " + identifier.to_string());
        }
        return m_entries_map.at(identifier);
    }

    void read(std::string_view base_path, std::string_view extra_path = "") {
        if (extra_path.empty()) std::cout << "parsing registry " << m_identifier.to_string() << " from " << base_path << "\n";
        // todo: replace with vfs
        if (!std::filesystem::exists(std::string(base_path)+ "/" + m_identifier.m_path + extra_path) ||
            !std::filesystem::is_directory(std::string(base_path) + "/" + m_identifier.m_path + extra_path)) return;

        for (const auto& entry : std::filesystem::directory_iterator(std::string(base_path) + "/" + m_identifier.m_path + extra_path)) {
            if (entry.is_directory()) {
                std::string new_extra_path = std::string(extra_path) + "/" + entry.path().filename().string();
                read(base_path, new_extra_path);
            } else {
                auto path = std::filesystem::path(base_path);
                std::string ns_str = path.filename().string();
                std::string path_str = (
                    extra_path.size() ? std::string(extra_path) + "/" + entry.path().filename().string() : entry.path().filename().string()
                );
                std::ifstream f (entry.path());
                std::string buffer (std::filesystem::file_size(entry.path()), ' ');
                f.read(buffer.data(), buffer.size());
                f.close();

                T result = nlohmann::json::parse(buffer).get<T>();
                register_object(resource_location_t(ns_str, path_str), std::make_shared<T>(std::move(result)));
            }
        }
    }
};

}
