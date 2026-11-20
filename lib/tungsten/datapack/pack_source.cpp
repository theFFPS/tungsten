/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/datapack/pack_source.hpp>

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <miniz.h>

namespace tungsten {

namespace {

std::string generic_string(const std::filesystem::path& path) {
    return path.generic_string();
}
bool is_safe_relative_path(std::string_view path) {
    if (path.empty() || path.front() == '/') return false;
    std::size_t pos = 0;
    while (pos <= path.size()) {
        std::size_t next = path.find('/', pos);
        std::string_view segment = path.substr(pos, next == std::string_view::npos ? std::string_view::npos : next - pos);
        if (segment == "..") return false;
        if (next == std::string_view::npos) break;
        pos = next + 1;
    }
    return true;
}

}

directory_pack_source_t::directory_pack_source_t(std::filesystem::path root) : m_root(std::move(root)) {
    if (!std::filesystem::is_directory(m_root)) {
        throw std::runtime_error("directory_pack_source_t error: '" + m_root.string() + "' is not a directory");
    }
}
std::string directory_pack_source_t::describe() const {
    return m_root.string();
}
const std::vector<std::string>& directory_pack_source_t::list_files() const {
    std::lock_guard lock (m_mutex);
    if (!m_listed) {
        std::error_code ec;
        for (std::filesystem::recursive_directory_iterator it (m_root, std::filesystem::directory_options::follow_directory_symlink, ec), end;
             !ec && it != end; it.increment(ec)) {
            if (!it->is_regular_file(ec)) continue;
            m_files.push_back(generic_string(std::filesystem::relative(it->path(), m_root, ec)));
        }
        std::sort(m_files.begin(), m_files.end());
        m_listed = true;
    }
    return m_files;
}
bool directory_pack_source_t::exists(std::string_view path) const {
    if (!is_safe_relative_path(path)) return false;
    std::error_code ec;
    return std::filesystem::is_regular_file(m_root / std::filesystem::path(std::string(path)), ec);
}
std::vector<std::uint8_t> directory_pack_source_t::read(std::string_view path) const {
    if (!is_safe_relative_path(path)) {
        throw std::runtime_error("directory_pack_source_t::read error: unsafe path '" + std::string(path) + "'");
    }
    auto full_path = m_root / std::filesystem::path(std::string(path));
    std::ifstream f (full_path, std::ios::binary);
    if (!f) throw std::runtime_error("directory_pack_source_t::read error: unable to open '" + full_path.string() + "'");
    f.seekg(0, std::ios::end);
    auto size = static_cast<std::size_t>(f.tellg());
    f.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> bytes (size);
    if (size != 0) f.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size));
    if (!f) throw std::runtime_error("directory_pack_source_t::read error: unable to read '" + full_path.string() + "'");
    return bytes;
}

struct zip_pack_source_t::impl_t {
    mz_zip_archive m_archive {};
    std::vector<std::uint8_t> m_memory; // backing storage for in-memory archives
    std::unordered_map<std::string, mz_uint> m_index;
    mutable std::mutex m_mutex;
    bool m_open = false;

    ~impl_t() {
        if (m_open) mz_zip_reader_end(&m_archive);
    }
};

zip_pack_source_t::zip_pack_source_t(const std::filesystem::path& path) : m_impl(std::make_unique<impl_t>()), m_description(path.string()) {
    if (!mz_zip_reader_init_file(&m_impl->m_archive, path.string().c_str(), 0)) {
        throw std::runtime_error("zip_pack_source_t error: unable to open '" + path.string() + "' as a zip archive: "
            + mz_zip_get_error_string(mz_zip_get_last_error(&m_impl->m_archive)));
    }
    m_impl->m_open = true;
    for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&m_impl->m_archive); ++i) {
        if (mz_zip_reader_is_file_a_directory(&m_impl->m_archive, i)) continue;
        mz_zip_archive_file_stat stat {};
        if (!mz_zip_reader_file_stat(&m_impl->m_archive, i, &stat)) continue;
        std::string name = stat.m_filename;
        std::replace(name.begin(), name.end(), '\\', '/');
        m_impl->m_index[name] = i;
        m_files.push_back(std::move(name));
    }
    std::sort(m_files.begin(), m_files.end());
}
zip_pack_source_t::zip_pack_source_t(std::vector<std::uint8_t> archive, std::string description)
    : m_impl(std::make_unique<impl_t>()), m_description(std::move(description)) {
    m_impl->m_memory = std::move(archive);
    if (!mz_zip_reader_init_mem(&m_impl->m_archive, m_impl->m_memory.data(), m_impl->m_memory.size(), 0)) {
        throw std::runtime_error("zip_pack_source_t error: unable to open '" + m_description + "' as a zip archive: "
            + mz_zip_get_error_string(mz_zip_get_last_error(&m_impl->m_archive)));
    }
    m_impl->m_open = true;
    for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&m_impl->m_archive); ++i) {
        if (mz_zip_reader_is_file_a_directory(&m_impl->m_archive, i)) continue;
        mz_zip_archive_file_stat stat {};
        if (!mz_zip_reader_file_stat(&m_impl->m_archive, i, &stat)) continue;
        std::string name = stat.m_filename;
        std::replace(name.begin(), name.end(), '\\', '/');
        m_impl->m_index[name] = i;
        m_files.push_back(std::move(name));
    }
    std::sort(m_files.begin(), m_files.end());
}
zip_pack_source_t::~zip_pack_source_t() = default;

std::string zip_pack_source_t::describe() const {
    return m_description;
}
const std::vector<std::string>& zip_pack_source_t::list_files() const {
    return m_files;
}
bool zip_pack_source_t::exists(std::string_view path) const {
    return m_impl->m_index.contains(std::string(path));
}
std::vector<std::uint8_t> zip_pack_source_t::read(std::string_view path) const {
    auto it = m_impl->m_index.find(std::string(path));
    if (it == m_impl->m_index.end()) {
        throw std::runtime_error("zip_pack_source_t::read error: '" + std::string(path) + "' not found in " + m_description);
    }
    std::lock_guard lock (m_impl->m_mutex);
    mz_zip_archive_file_stat stat {};
    if (!mz_zip_reader_file_stat(&m_impl->m_archive, it->second, &stat)) {
        throw std::runtime_error("zip_pack_source_t::read error: unable to stat '" + std::string(path) + "' in " + m_description);
    }
    std::vector<std::uint8_t> bytes (static_cast<std::size_t>(stat.m_uncomp_size));
    if (!bytes.empty() && !mz_zip_reader_extract_to_mem(&m_impl->m_archive, it->second, bytes.data(), bytes.size(), 0)) {
        throw std::runtime_error("zip_pack_source_t::read error: unable to extract '" + std::string(path) + "' from " + m_description + ": "
            + mz_zip_get_error_string(mz_zip_get_last_error(&m_impl->m_archive)));
    }
    return bytes;
}

std::shared_ptr<pack_source_t> open_pack_source(const std::filesystem::path& path) {
    if (std::filesystem::is_directory(path)) return std::make_shared<directory_pack_source_t>(path);
    if (std::filesystem::is_regular_file(path)) return std::make_shared<zip_pack_source_t>(path);
    throw std::runtime_error("open_pack_source error: '" + path.string() + "' is neither a directory nor a file");
}

}
