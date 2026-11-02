/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <cstdint>
#include <vector>
#include <string_view>
#include <memory>
#include "vfs_struct.hpp"

namespace tungsten {

struct ramfs_inode_t {
    uint64_t m_ino;
    uint64_t m_ctime;
    uint64_t m_mtime;
    uint64_t m_atime;
    uint32_t m_nlink;

    std::vector<uint8_t> m_bytes;
};
struct ramfs_t final : public vfs_filesystem_t {
private:
    std::shared_ptr<vfs_dirent_t> m_root;
    std::string m_root_path;

    std::vector<ramfs_inode_t> m_inodes;
    std::vector<size_t> m_available_inodes;
public:
    ramfs_t() {
        m_root = std::make_shared<vfs_dirent_t>("/", vfs_filetype_t::DIR);
        m_root->m_nid = 0;

        uint64_t ctime = static_cast<uint64_t>(time(nullptr));
        m_inodes.push_back(ramfs_inode_t{
            .m_ino = 0,
            .m_ctime = ctime,
            .m_mtime = ctime,
            .m_atime = ctime,
            .m_nlink = 1,
            .m_bytes{}
        });
    }

    inline void parse() override {}
    void form_dirent_tree(std::shared_ptr<vfs_dirent_t> dest) override;
    void set_root(std::string_view path) override;
    vfs_mount_flags_t mount_flags() const override;
    std::vector<std::string> mount_options() const override;
    std::string_view get_root() const override;

    size_t file_size(std::string_view, std::shared_ptr<vfs_dirent_t>) override;
    vfs_fileinfo_t file_info(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) override;

    bool read(std::string_view path, uint8_t* dest, size_t length, std::shared_ptr<vfs_dirent_t> dirent) override;
    bool read_at(std::string_view path, size_t offset, uint8_t* dest, size_t length, std::shared_ptr<vfs_dirent_t> dirent) override;
    void write(std::string_view, const uint8_t*, size_t, std::shared_ptr<vfs_dirent_t>) override;
    void write_at(std::string_view, size_t, const uint8_t*, size_t, std::shared_ptr<vfs_dirent_t>) override;

    void mkdir(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent_holder) override;
    void touch(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent_holder) override;
    void symlink(std::string_view path, std::string_view target, std::shared_ptr<vfs_dirent_t> dirent_holder) override;
    void rmdir(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) override;
    void rm(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) override;
    void hardlink(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent, std::shared_ptr<vfs_dirent_t> dirent_holder) override;
};

}
