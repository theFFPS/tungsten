/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace tungsten {

// read-only container of files addressed by forward-slash relative paths ("data/minecraft/recipe/stick.json")
struct pack_source_t {
    virtual ~pack_source_t() = default;

    virtual std::string describe() const = 0;
    // all regular files, sorted, relative, '/' separated
    virtual const std::vector<std::string>& list_files() const = 0;
    virtual bool exists(std::string_view path) const = 0;
    // throws std::runtime_error when the file does not exist or can not be read
    virtual std::vector<std::uint8_t> read(std::string_view path) const = 0;
};

struct directory_pack_source_t final : public pack_source_t {
private:
    std::filesystem::path m_root;
    mutable std::vector<std::string> m_files;
    mutable bool m_listed = false;
    mutable std::mutex m_mutex;
public:
    explicit directory_pack_source_t(std::filesystem::path root);

    std::string describe() const override;
    const std::vector<std::string>& list_files() const override;
    bool exists(std::string_view path) const override;
    std::vector<std::uint8_t> read(std::string_view path) const override;
};

struct zip_pack_source_t final : public pack_source_t {
private:
    struct impl_t;
    std::unique_ptr<impl_t> m_impl;
    std::string m_description;
    std::vector<std::string> m_files;
public:
    explicit zip_pack_source_t(const std::filesystem::path& path);
    // takes a copy of the archive bytes
    zip_pack_source_t(std::vector<std::uint8_t> archive, std::string description);
    ~zip_pack_source_t() override;

    std::string describe() const override;
    const std::vector<std::string>& list_files() const override;
    bool exists(std::string_view path) const override;
    std::vector<std::uint8_t> read(std::string_view path) const override;
};

// a directory or a .zip archive
std::shared_ptr<pack_source_t> open_pack_source(const std::filesystem::path& path);

}
