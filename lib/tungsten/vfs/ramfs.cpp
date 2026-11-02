/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/vfs/ramfs.hpp>
#include <tungsten/vfs/vfs.hpp>

namespace tungsten {

void ramfs_t::form_dirent_tree(std::shared_ptr<vfs_dirent_t> dest) {
    dest->m_children = m_root->m_children;
    dest->m_filetype = m_root->m_filetype;
    dest->m_nid = m_root->m_nid;
    dest->m_symlink_target_path = m_root->m_symlink_target_path;
}
void ramfs_t::set_root(std::string_view path) { m_root_path = path; }
vfs_mount_flags_t ramfs_t::mount_flags() const {
    return g_vfs->get_mount_info(m_root_path).m_flags;
}
std::vector<std::string> ramfs_t::mount_options() const {
    return g_vfs->get_mount_info(m_root_path).m_options;
}
std::string_view ramfs_t::get_root() const {
    return m_root_path;
}

size_t ramfs_t::file_size(std::string_view, std::shared_ptr<vfs_dirent_t> dirent) {
    ramfs_inode_t& inode = m_inodes[dirent->m_nid];
    return inode.m_bytes.size();
}
vfs_fileinfo_t ramfs_t::file_info(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) {
    ramfs_inode_t& inode = m_inodes[dirent->m_nid];
    return vfs_fileinfo_t{
        .m_path = std::string(path),
        .m_nid = dirent->m_nid,

        .m_filetype = dirent->m_filetype,

        .m_ctime = inode.m_ctime,
        .m_mtime = inode.m_mtime,
        .m_atime = inode.m_atime,
    };
}

bool ramfs_t::read(std::string_view path, uint8_t* dest, size_t length, std::shared_ptr<vfs_dirent_t> dirent) {
    ramfs_inode_t& inode = m_inodes[dirent->m_nid];
    inode.m_atime = time(nullptr);
    if (dirent->m_filetype != vfs_filetype_t::REG) {
        throw std::runtime_error("ramfs_t::read error: can only read 'regular' files");
    }
    if (length > file_size(path, dirent)) {
        throw std::runtime_error("ramfs_t::read error: out of bounds.");
    }
    std::copy(inode.m_bytes.begin(), inode.m_bytes.begin() + length, dest);
    return true;
}
bool ramfs_t::read_at(std::string_view path, size_t offset, uint8_t* dest, size_t length, std::shared_ptr<vfs_dirent_t> dirent) {
    ramfs_inode_t& inode = m_inodes[dirent->m_nid];
    inode.m_atime = time(nullptr);
    if (dirent->m_filetype != vfs_filetype_t::REG) {
        throw std::runtime_error("ramfs_t::read_at error: can only read 'regular' files");
    }
    if (length + offset > file_size(path, dirent)) {
        throw std::runtime_error("ramfs_t::read_at error: out of bounds.");
    }
    std::copy(inode.m_bytes.begin() + offset, inode.m_bytes.begin() + offset + length, dest);
    return true;
}
void ramfs_t::write(std::string_view, const uint8_t* src, size_t length, std::shared_ptr<vfs_dirent_t> dirent) {
    if (static_cast<uint16_t>(mount_flags()) & static_cast<uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
        throw std::runtime_error("ramfs_t::write error: " + std::string(m_root_path) + " is read-only");
    }
    ramfs_inode_t& inode = m_inodes[dirent->m_nid];
    inode.m_atime = time(nullptr);
    inode.m_mtime = inode.m_atime;
    if (dirent->m_filetype != vfs_filetype_t::REG) {
        throw std::runtime_error("ramfs_t::write error: can only read 'regular' files");
    }
    inode.m_bytes.resize(length);
    std::copy(src, src + length, inode.m_bytes.begin());
}
void ramfs_t::write_at(std::string_view, size_t offset, const uint8_t* src, size_t length, std::shared_ptr<vfs_dirent_t> dirent) {
    if (static_cast<uint16_t>(mount_flags()) & static_cast<uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
        throw std::runtime_error("ramfs_t::write_at error: " + std::string(m_root_path) + " is read-only");
    }
    ramfs_inode_t& inode = m_inodes[dirent->m_nid];
    inode.m_atime = time(nullptr);
    inode.m_mtime = inode.m_atime;
    if (dirent->m_filetype != vfs_filetype_t::REG) {
        throw std::runtime_error("ramfs_t::write_at error: can only read 'regular' files");
    }
    size_t required_size = offset + length;
    if (required_size > inode.m_bytes.size()) {
        inode.m_bytes.resize(required_size);
    }
    std::copy(src, src + length, inode.m_bytes.begin() + offset);
}

void ramfs_t::mkdir(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent_holder) {
    if (static_cast<uint16_t>(mount_flags()) & static_cast<uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
        throw std::runtime_error("ramfs_t::mkdir error: " + std::string(m_root_path) + " is read-only");
    }
    vfs_fileinfo_t file_info = g_vfs->file_info(g_vfs->normalize_absolute(std::string(path) + "/.."));
    dirent_holder->m_nid = m_inodes.size();
    uint64_t ctime = static_cast<uint64_t>(time(nullptr));
    if (m_available_inodes.empty()) m_inodes.push_back(ramfs_inode_t{
        .m_ino = dirent_holder->m_nid,
        .m_ctime = ctime,
        .m_mtime = ctime,
        .m_atime = ctime,
        .m_nlink = 1,
        .m_bytes{}
    }); else {
        size_t nid = m_available_inodes.back();
        m_available_inodes.pop_back();
        m_inodes[nid] = ramfs_inode_t{
            .m_ino = dirent_holder->m_nid,
            .m_ctime = ctime,
            .m_mtime = ctime,
            .m_atime = ctime,
            .m_nlink = 1,
            .m_bytes{}
        };
    }
}
void ramfs_t::touch(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent_holder) {
    if (static_cast<uint16_t>(mount_flags()) & static_cast<uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
        throw std::runtime_error("ramfs_t::touch error: " + std::string(m_root_path) + " is read-only");
    }
    vfs_fileinfo_t file_info = g_vfs->file_info(g_vfs->normalize_absolute(std::string(path) + "/.."));
    dirent_holder->m_nid = m_inodes.size();
    uint64_t ctime = static_cast<uint64_t>(time(nullptr));
    if (m_available_inodes.empty()) m_inodes.push_back(ramfs_inode_t{
        .m_ino = dirent_holder->m_nid,
        .m_ctime = ctime,
        .m_mtime = ctime,
        .m_atime = ctime,
        .m_nlink = 1,
        .m_bytes{}
    }); else {
        size_t nid = m_available_inodes.back();
        m_available_inodes.pop_back();
        m_inodes[nid] = ramfs_inode_t{
            .m_ino = dirent_holder->m_nid,
            .m_ctime = ctime,
            .m_mtime = ctime,
            .m_atime = ctime,
            .m_nlink = 1,
            .m_bytes{}
        };
    }
}
void ramfs_t::symlink(std::string_view path, std::string_view, std::shared_ptr<vfs_dirent_t> dirent_holder) {
    if (static_cast<uint16_t>(mount_flags()) & static_cast<uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
        throw std::runtime_error("ramfs_t::symlink error: " + std::string(m_root_path) + " is read-only");
    }
    vfs_fileinfo_t file_info = g_vfs->file_info(g_vfs->normalize_absolute(std::string(path) + "/.."));
    dirent_holder->m_nid = m_inodes.size();
    uint64_t ctime = static_cast<uint64_t>(time(nullptr));
    if (m_available_inodes.empty()) m_inodes.push_back(ramfs_inode_t{
        .m_ino = dirent_holder->m_nid,
        .m_ctime = ctime,
        .m_mtime = ctime,
        .m_atime = ctime,
        .m_nlink = 1,
        .m_bytes{}
    }); else {
        size_t nid = m_available_inodes.back();
        m_available_inodes.pop_back();
        m_inodes[nid] = ramfs_inode_t{
            .m_ino = dirent_holder->m_nid,
            .m_ctime = ctime,
            .m_mtime = ctime,
            .m_atime = ctime,
            .m_nlink = 1,
            .m_bytes{}
        };
    }
}
void ramfs_t::rmdir(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) {
    if (static_cast<uint16_t>(mount_flags()) & static_cast<uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
        throw std::runtime_error("ramfs_t::rmdir error: " + std::string(m_root_path) + " is read-only");
    }
    if (dirent->m_children.size()) {
        throw std::runtime_error("ramfs_t::rmdir error: " + std::string(m_root_path) + " must be empty");
    }
    auto parent_dirent = g_vfs->read_dir(g_vfs->normalize_absolute(std::string(path) + "/.."));
    auto inode = m_inodes[dirent->m_nid];
    inode.m_nlink--;
    if (!inode.m_nlink) m_available_inodes.push_back(dirent->m_nid);
    parent_dirent->m_children.erase(std::string(dirent->m_path_segment));
}
void ramfs_t::rm(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) {
    if (static_cast<uint16_t>(mount_flags()) & static_cast<uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
        throw std::runtime_error("ramfs_t::rm error: " + std::string(m_root_path) + " is read-only");
    }
    auto parent_dirent = g_vfs->read_dir(g_vfs->normalize_absolute(std::string(path) + "/.."));
    auto inode = m_inodes[dirent->m_nid];
    inode.m_nlink--;
    if (!inode.m_nlink) m_available_inodes.push_back(dirent->m_nid);
    parent_dirent->m_children.erase(std::string(dirent->m_path_segment));
}
void ramfs_t::hardlink(std::string_view, std::shared_ptr<vfs_dirent_t> dirent, std::shared_ptr<vfs_dirent_t> dirent_holder) {
    if (static_cast<uint16_t>(mount_flags()) & static_cast<uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
        throw std::runtime_error("ramfs_t::hardlink error: " + std::string(m_root_path) + " is read-only");
    }
    dirent_holder->m_nid = dirent->m_nid;
    dirent_holder->m_filetype = dirent->m_filetype;
    dirent_holder->m_children = dirent->m_children;
    dirent_holder->m_is_mountpoint = dirent->m_is_mountpoint;
    auto inode = m_inodes[dirent->m_nid];
    inode.m_nlink++;
}

}
