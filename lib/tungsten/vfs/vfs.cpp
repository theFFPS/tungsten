/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/vfs/vfs.hpp>
#include <tungsten/vfs/ramfs.hpp>
#include <mutex>

namespace tungsten {

std::shared_ptr<vfs_t> g_vfs;

void correct_dirent_filesystem(std::shared_ptr<vfs_dirent_t> dirent, std::shared_ptr<vfs_filesystem_t> file_system) {
    for (auto child_dirent : dirent->m_children) {
        child_dirent.second->m_file_system = file_system;
        if (child_dirent.second->m_children.size()) correct_dirent_filesystem(child_dirent.second, file_system);
    }
}

std::shared_ptr<vfs_dirent_t> vfs_t::read_dir(std::string_view path, uint16_t depth) {
    if (!m_root->m_is_mountpoint) return m_root;
    if (depth > 512) throw std::runtime_error("vfs_t::read_dir error: recursion depth reached");
    std::vector<std::string> path_components = get_path_components(path);
    std::shared_ptr<vfs_dirent_t> dirent = m_root;
    for (const auto& path_component : path_components) {
        if (path_component.empty()) continue;
        if (!dirent->m_children.contains(path_component)) {
            throw std::runtime_error("vfs_t::read_dir error: " + std::string(path) + " does not exist");
        }
        if (dirent->m_children[path_component]->m_filetype == vfs_filetype_t::LNK) {
            try {
                dirent = read_dir(dirent->m_children[path_component]->m_symlink_target_path, depth + 1);
            } catch (std::exception& e) {
                return dirent->m_children[path_component];
            }
        } else dirent = dirent->m_children[path_component];
    }
    return dirent;
}
void vfs_t::initialize() {
    mount("/", std::make_shared<ramfs_t>());
    mkdir("/mnt");
}

void vfs_t::mount(std::string_view path, std::shared_ptr<vfs_filesystem_t> file_system, vfs_mount_flags_t flags, const std::vector<std::string>& options) {
    std::unique_lock lock (m_mutex);

    if (m_directories_in_use.contains(path) && m_directories_in_use[path]) throw std::runtime_error("vfs_t::mount error: " + std::string(path) + " is in use");

    std::shared_ptr<vfs_dirent_t> dirent = read_dir(path);
    if (dirent->m_is_mountpoint) throw std::runtime_error("vfs_t::mount error: " + std::string(path) + " is already mounted");
    if (dirent->m_children.size()) throw std::runtime_error("vfs_t::mount error: " + std::string(path) + " is not empty");
    m_mountpoints.insert({ path, vfs_mountpoint_t{
        .m_path = std::string(path),
        .m_path_device = "/dev/null",
        .m_file_system = file_system,
        .m_flags = flags,
        .m_options = options
    } });
    m_mountpoint_infos.insert({ path, vfs_mountpoint_info_t{ .m_path = std::string(path), .m_flags = flags, .m_options = options } });
    dirent->m_is_mountpoint = true;
    dirent->m_file_system = file_system;
    file_system->set_root(path);
    file_system->parse();
    file_system->form_dirent_tree(dirent);
    correct_dirent_filesystem(dirent, file_system);
}
void vfs_t::umount(std::string_view path) {
    std::unique_lock lock (m_mutex);

    if (m_directories_in_use.contains(path)) {
        if (m_directories_in_use[path]) throw std::runtime_error("vfs_t::umount error: " + std::string(path) + " is in use");
    }

    std::shared_ptr<vfs_dirent_t> dirent = read_dir(path);
    if (!dirent->m_is_mountpoint) throw std::runtime_error("vfs_t::umount error: " + std::string(path) + " is not mountpoint");
    dirent->m_children.clear();
    dirent->m_is_mountpoint = false;
    m_mountpoints.erase(path);
}
void vfs_t::umount_all() {
    std::unique_lock lock (m_mutex);

    m_root->m_children.clear();
    m_root->m_is_mountpoint = false;
    m_root->m_symlink_target_path = "";
    m_root->m_path_segment = "/";

    m_mountpoints.clear();
}

bool vfs_t::exists(std::string_view path) {
    try {
        std::shared_lock lock (m_mutex);
        return read_dir(path) != nullptr;
    } catch (std::exception&) {
        return false;
    }
}
size_t vfs_t::file_size(std::string_view path) {
    std::shared_lock lock (m_mutex);
    auto dirent = read_dir(path);
    return dirent->m_file_system.lock()->file_size(path, dirent);
}
vfs_filetype_t vfs_t::filetype(std::string_view path) {
    std::shared_lock lock (m_mutex);
    return read_dir(path)->m_filetype;
}
vfs_fileinfo_t vfs_t::file_info(std::string_view path) {
    std::shared_lock lock (m_mutex);
    auto dirent = read_dir(path);
    return dirent->m_file_system.lock()->file_info(path, dirent);
}
bool vfs_t::is_dir(std::string_view path) {
    return filetype(path) == vfs_filetype_t::DIR;
}
bool vfs_t::is_symlink(std::string_view path) {
    return filetype(path) == vfs_filetype_t::LNK;
}
bool vfs_t::is_mountpoint(std::string_view path) {
    std::shared_lock lock (m_mutex);
    return read_dir(path)->m_is_mountpoint;
}
vfs_mountpoint_info_t vfs_t::get_mount_info(std::string_view path) {
    std::shared_lock lock (m_mutex);
    if (!read_dir(path)->m_is_mountpoint) {
        throw std::runtime_error("vfs_t::get_mount_info error: " + std::string(path) + " is not a mountpoint");
    }
    return m_mountpoint_infos[path];
}

bool vfs_t::read(std::string_view path, uint8_t* dest, size_t length) {
    std::shared_ptr<vfs_dirent_t> dirent;
    {
        std::shared_lock lock (m_mutex);
        dirent = read_dir(path);
    }
    return dirent->m_file_system.lock()->read(path, dest, length, dirent);
}
bool vfs_t::read_at(std::string_view path, size_t offset, uint8_t* dest, size_t length) {
    std::shared_ptr<vfs_dirent_t> dirent;
    {
        std::shared_lock lock (m_mutex);
        dirent = read_dir(path);
    }
    return dirent->m_file_system.lock()->read_at(path, offset, dest, length, dirent);
}
void vfs_t::write(std::string_view path, const uint8_t* src, size_t length) {
    std::shared_ptr<vfs_dirent_t> dirent;
    {
        std::shared_lock lock (m_mutex);
        dirent = read_dir(path);
    }
    if (static_cast<uint16_t>(dirent->m_file_system.lock()->mount_flags()) & static_cast<uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
        throw std::runtime_error("vfs_t::write error: " + std::string(path) + " is read only.");
    }
    dirent->m_file_system.lock()->write(path, src, length, dirent);
}
void vfs_t::write_at(std::string_view path, size_t offset, const uint8_t* src, size_t length) {
    std::shared_ptr<vfs_dirent_t> dirent;
    {
        std::shared_lock lock (m_mutex);
        dirent = read_dir(path);
    }
    if (static_cast<uint16_t>(dirent->m_file_system.lock()->mount_flags()) & static_cast<uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
        throw std::runtime_error("vfs_t::write_at error: " + std::string(path) + " is read only.");
    }
    dirent->m_file_system.lock()->write_at(path, offset, src, length, dirent);
}

void vfs_t::mkdir(std::string_view path) {
    std::shared_ptr<vfs_dirent_t> parent_dirent;
    std::string parent_path = normalize_absolute(std::string(path) + "/..");
    {
        std::shared_lock lock (m_mutex);
        parent_dirent = read_dir(parent_path);
    }
    std::shared_ptr<vfs_dirent_t> dirent_holder = std::make_shared<vfs_dirent_t>(
        get_path_components(path).back(), vfs_filetype_t::DIR
    );
    dirent_holder->m_file_system = parent_dirent->m_file_system.lock();
    parent_dirent->m_children.insert({ std::string(dirent_holder->m_path_segment), dirent_holder });
    parent_dirent->m_file_system.lock()->mkdir(path, dirent_holder);
}
void vfs_t::touch(std::string_view path) {
    std::shared_ptr<vfs_dirent_t> parent_dirent;
    std::string parent_path = normalize_absolute(std::string(path) + "/..");
    {
        std::shared_lock lock (m_mutex);
        parent_dirent = read_dir(parent_path);
    }
    std::shared_ptr<vfs_dirent_t> dirent_holder = std::make_shared<vfs_dirent_t>(
        get_path_components(path).back(), vfs_filetype_t::REG
    );
    dirent_holder->m_file_system = parent_dirent->m_file_system.lock();
    parent_dirent->m_children.insert({ std::string(dirent_holder->m_path_segment), dirent_holder });
    parent_dirent->m_file_system.lock()->touch(path, dirent_holder);
}
void vfs_t::symlink(std::string_view path, std::string_view target) {
    std::shared_ptr<vfs_dirent_t> parent_dirent;
    std::string parent_path = normalize_absolute(std::string(path) + "/..");
    {
        std::shared_lock lock (m_mutex);
        parent_dirent = read_dir(parent_path);
    }
    std::shared_ptr<vfs_dirent_t> dirent_holder = std::make_shared<vfs_dirent_t>(
        get_path_components(path).back(), vfs_filetype_t::LNK, target
    );
    dirent_holder->m_file_system = parent_dirent->m_file_system.lock();
    parent_dirent->m_children.insert({ std::string(dirent_holder->m_path_segment), dirent_holder });
    parent_dirent->m_file_system.lock()->symlink(path, target, dirent_holder);
}
void vfs_t::rmdir(std::string_view path) {
    std::shared_ptr<vfs_dirent_t> dirent;
    {
        std::shared_lock lock (m_mutex);
        dirent = read_dir(path);
    }
    dirent->m_file_system.lock()->rmdir(path, dirent);
}
void vfs_t::rm(std::string_view path) {
    std::shared_ptr<vfs_dirent_t> dirent;
    {
        std::shared_lock lock (m_mutex);
        dirent = read_dir(path);
    }
    dirent->m_file_system.lock()->rm(path, dirent);
}
void vfs_t::hardlink(std::string_view path, std::string_view target) {
    std::shared_ptr<vfs_dirent_t> parent_dirent, dirent;
    std::string parent_path = normalize_absolute(std::string(path) + "/..");
    {
        std::shared_lock lock (m_mutex);
        parent_dirent = read_dir(parent_path);
        dirent = read_dir(target);
    }
    std::shared_ptr<vfs_dirent_t> dirent_holder = std::make_shared<vfs_dirent_t>(
        get_path_components(path).back(), vfs_filetype_t::LNK, target
    );
    dirent_holder->m_file_system = parent_dirent->m_file_system.lock();
    parent_dirent->m_children.insert({ std::string(dirent_holder->m_path_segment), dirent_holder });
    parent_dirent->m_file_system.lock()->hardlink(path, dirent, dirent_holder);
}

std::vector<std::string> vfs_t::get_path_components(std::string_view path) {
    std::vector<std::string> path_components;
    std::string path_component_temp;
    for (const auto& c : path) {
        if (c == '/') {
            path_components.push_back(path_component_temp);
            path_component_temp.clear();
        } else path_component_temp += c;
    }
    if (!path.ends_with('/')) path_components.push_back(path_component_temp);
    return path_components;
}
bool vfs_t::is_absolute(std::string_view path) {
    if (path.empty()) return false;
    return path.starts_with('/');
}
std::string vfs_t::to_absolute(std::string_view base, std::string_view path) {
    if (!is_absolute(base)) throw std::runtime_error("vfs_t::to_absolute error: base path is not absolute");
    if (is_absolute(path)) return std::string(path);
    return std::string(base) + "/" + std::string(path);
}
std::string vfs_t::normalize_absolute(std::string_view path) {
    if (!is_absolute(path)) throw std::runtime_error("vfs_t::normalize_absolute error: path is not absolute");
    std::vector<std::string> path_components = get_path_components(path);
    std::vector<std::string_view> normalized_path_components;
    for (const auto& path_component : path_components) {
        if (path_component.empty() || path_component == ".") continue;
        if (path_component == ".." && normalized_path_components.size()) {
            normalized_path_components.pop_back();
        } else normalized_path_components.push_back(path_component);
    }
    std::string normalized_path;
    for (const auto& path_component : normalized_path_components) {
        normalized_path += "/" + std::string(path_component);
    }
    if (normalized_path.empty()) return "/";
    return normalized_path;
}

}
