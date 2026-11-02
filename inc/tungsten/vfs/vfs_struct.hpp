/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <vector>
#include "vfs_fileinfo.hpp"

namespace tungsten {

struct vfs_filesystem_t;
struct vfs_dirent_t {
    std::string m_path_segment;
    std::unordered_map<std::string, std::shared_ptr<vfs_dirent_t>> m_children;

    bool m_is_mountpoint = false;
    std::weak_ptr<vfs_filesystem_t> m_file_system;

    vfs_filetype_t m_filetype = vfs_filetype_t::UNKNOWN;
    uint64_t m_nid;
    std::string m_symlink_target_path;

    vfs_dirent_t() {}
    vfs_dirent_t(const std::string& path_segment, vfs_filetype_t filetype) : m_path_segment(path_segment), m_filetype(filetype) {}
    vfs_dirent_t(const std::string& path_segment, vfs_filetype_t filetype, std::string_view symlink_target_path)
        : m_path_segment(path_segment), m_filetype(filetype), m_symlink_target_path(symlink_target_path) {}
    vfs_dirent_t(const std::string& path_segment, const std::unordered_map<std::string, std::shared_ptr<vfs_dirent_t>>& children, vfs_filetype_t filetype)
        : m_path_segment(path_segment), m_children(children), m_filetype(filetype) {}
};
struct vfs_filesystem_t {
    virtual ~vfs_filesystem_t() = default;

    virtual void parse() = 0;

    virtual void form_dirent_tree(std::shared_ptr<vfs_dirent_t> dest) = 0;
    virtual void set_root(std::string_view path) = 0;
    virtual vfs_mount_flags_t mount_flags() const = 0;
    virtual std::vector<std::string> mount_options() const = 0;
    virtual std::string_view get_root() const = 0;

    virtual size_t file_size(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) = 0;
    virtual vfs_fileinfo_t file_info(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) = 0;

    virtual bool read(std::string_view path, uint8_t* dest, size_t length, std::shared_ptr<vfs_dirent_t> dirent) = 0;
    virtual bool read_at(std::string_view path, size_t offset, uint8_t* dest, size_t length, std::shared_ptr<vfs_dirent_t> dirent) = 0;
    virtual void write(std::string_view path, const uint8_t* src, size_t length, std::shared_ptr<vfs_dirent_t> dirent) = 0;
    virtual void write_at(std::string_view path, size_t offset, const uint8_t* src, size_t length, std::shared_ptr<vfs_dirent_t> dirent) = 0;

    virtual void mkdir(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent_holder) = 0;
    virtual void touch(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent_holder) = 0;
    virtual void symlink(std::string_view path, std::string_view target, std::shared_ptr<vfs_dirent_t> dirent_holder) = 0;
    virtual void rmdir(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) = 0;
    virtual void rm(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) = 0;
    virtual void hardlink(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent, std::shared_ptr<vfs_dirent_t> dirent_holder) = 0;
};
struct vfs_mountpoint_info_t {
    std::string m_path;
    vfs_mount_flags_t m_flags;
    std::vector<std::string> m_options;
};
struct vfs_mountpoint_t {
    std::string m_path;
    std::string m_path_device;
    std::shared_ptr<vfs_filesystem_t> m_file_system;
    vfs_mount_flags_t m_flags;
    std::vector<std::string> m_options;
};

}
