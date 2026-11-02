/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include "vfs_struct.hpp"
#include "vfs_fileinfo.hpp"
#include <shared_mutex>
#include <tungsten/util/file_io.hpp>

namespace tungsten {

struct vfs_t {
private:
    std::shared_ptr<vfs_dirent_t> m_root;
    std::unordered_map<std::string_view, vfs_mountpoint_t> m_mountpoints;
    std::unordered_map<std::string_view, vfs_mountpoint_info_t> m_mountpoint_infos;
    std::unordered_map<std::string_view, bool> m_directories_in_use;
    std::shared_mutex m_mutex;
public:
    std::shared_ptr<vfs_dirent_t> read_dir(std::string_view path, uint16_t depth = 0);
    void initialize();

    vfs_t() {
        m_root = std::make_shared<vfs_dirent_t>("/", vfs_filetype_t::DIR);
    }
    ~vfs_t() {
        umount_all();
    }

    void mount(std::string_view path, std::shared_ptr<vfs_filesystem_t> file_system,
                vfs_mount_flags_t flags = vfs_mount_flags_t::DEFAULTS, const std::vector<std::string>& options = {});
    void umount(std::string_view path);
    void umount_all();

    bool exists(std::string_view path);
    size_t file_size(std::string_view path);
    vfs_fileinfo_t file_info(std::string_view path);
    vfs_filetype_t filetype(std::string_view path);
    bool is_dir(std::string_view path);
    bool is_symlink(std::string_view path);
    bool is_mountpoint(std::string_view path);
    vfs_mountpoint_info_t get_mount_info(std::string_view path);

    bool read(std::string_view path, uint8_t* dest, size_t length);
    bool read_at(std::string_view path, size_t offset, uint8_t* dest, size_t length);
    void write(std::string_view path, const uint8_t* src, size_t length);
    void write_at(std::string_view path, size_t offset, const uint8_t* src, size_t length);

    void mkdir(std::string_view path);
    void touch(std::string_view path);
    void symlink(std::string_view path, std::string_view target);
    void rmdir(std::string_view path);
    void rm(std::string_view path);
    void hardlink(std::string_view path, std::string_view target);

    static std::vector<std::string> get_path_components(std::string_view path);
    static bool is_absolute(std::string_view path);
    static std::string to_absolute(std::string_view base, std::string_view path);
    static std::string normalize_absolute(std::string_view path);
};

struct vfs_file_handle_t final : public file_handle_t {
private:
    std::string m_path;
    std::ios::openmode m_mode;
    std::atomic<size_t> m_offset;
public:
    vfs_file_handle_t() : m_mode(std::ios::ate) {
        m_offset.store(0);
    }
    vfs_file_handle_t(std::string_view path, std::ios::openmode mode = std::ios::in | std::ios::out) {
        open(path, mode);
    }
    ~vfs_file_handle_t() {
        close();
    }

    vfs_file_handle_t(const vfs_file_handle_t&) = delete;
    void operator=(const vfs_file_handle_t&) = delete;

    vfs_file_handle_t(vfs_file_handle_t&& other) = delete;
    void operator=(vfs_file_handle_t&& other) = delete;

    bool open(std::string_view path, std::ios::openmode flags) override;
    void close() override;

    bool is_open() const override;
    bool can_read() const override;
    bool can_write() const override;
    std::ios::openmode mode() const override;
    std::string_view path() const override;
    size_t size() const override;

    bool read(uint8_t* dest, size_t length) override;
    bool read_at(size_t pos, uint8_t* dest, size_t length) override;
    void write(const uint8_t* src, size_t length) override;
    void write_at(size_t pos, const uint8_t* src, size_t length) override;

    size_t tell() const override;
    size_t seek(int64_t pos, file_io_whence_t whence = file_io_whence_t::START) override;
};

extern std::shared_ptr<vfs_t> g_vfs;

}
