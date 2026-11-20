/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

export module tungsten.vfs;

import std;
import tungsten.io;

export namespace tungsten {

enum class vfs_filetype_t : std::uint8_t {
    UNKNOWN = 0,
    REG = 1,
    DIR = 2,
    LNK = 3
};
enum class vfs_mount_flags_t : std::uint16_t {
    READ_ONLY    = 1,
    EXEC         = 2,
    DEV          = 4,
    SUID         = 8,
    ASYNC        = 16,
    ATIME        = 32,
    DIR_ATIME    = 64,
    REL_ATIME    = 128,
    PSEUDO       = 256,

    DEFAULTS = SUID | DEV | EXEC | ASYNC,
    DEFAULTS_RELATIME = DEFAULTS | REL_ATIME,
    DEFAULTS_RO = READ_ONLY | SUID | EXEC | ASYNC
};

struct vfs_fileinfo_t {
    std::string m_path;
    std::uint64_t m_nid;

    vfs_filetype_t m_filetype;

    std::uint64_t m_ctime;
    std::uint64_t m_mtime;
    std::uint64_t m_atime;
};

struct vfs_filesystem_t;
struct vfs_dirent_t {
    std::string m_path_segment;
    std::unordered_map<std::string, std::shared_ptr<vfs_dirent_t>> m_children;

    bool m_is_mountpoint = false;
    std::weak_ptr<vfs_filesystem_t> m_file_system;

    vfs_filetype_t m_filetype = vfs_filetype_t::UNKNOWN;
    std::uint64_t m_nid;
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

    virtual std::size_t file_size(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) = 0;
    virtual vfs_fileinfo_t file_info(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) = 0;

    virtual bool read(std::string_view path, std::uint8_t* dest, std::size_t length, std::shared_ptr<vfs_dirent_t> dirent) = 0;
    virtual bool read_at(std::string_view path, std::size_t offset, std::uint8_t* dest, std::size_t length, std::shared_ptr<vfs_dirent_t> dirent) = 0;
    virtual void write(std::string_view path, const std::uint8_t* src, std::size_t length, std::shared_ptr<vfs_dirent_t> dirent) = 0;
    virtual void write_at(std::string_view path, std::size_t offset, const std::uint8_t* src, std::size_t length, std::shared_ptr<vfs_dirent_t> dirent) = 0;

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

struct ramfs_inode_t {
    std::uint64_t m_ino;
    std::uint64_t m_ctime;
    std::uint64_t m_mtime;
    std::uint64_t m_atime;
    std::uint32_t m_nlink;

    std::vector<std::uint8_t> m_bytes;
};

struct vfs_t {
private:
    std::shared_ptr<vfs_dirent_t> m_root;
    std::unordered_map<std::string_view, vfs_mountpoint_t> m_mountpoints;
    std::unordered_map<std::string_view, vfs_mountpoint_info_t> m_mountpoint_infos;
    std::unordered_map<std::string_view, bool> m_directories_in_use;
    std::shared_mutex m_mutex;

    static void correct_dirent_filesystem(std::shared_ptr<vfs_dirent_t> dirent, std::shared_ptr<vfs_filesystem_t> file_system) {
        for (auto child_dirent : dirent->m_children) {
            child_dirent.second->m_file_system = file_system;
            if (child_dirent.second->m_children.size()) correct_dirent_filesystem(child_dirent.second, file_system);
        }
    }
public:
    std::shared_ptr<vfs_dirent_t> read_dir(std::string_view path, std::uint16_t depth = 0) {
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
    void initialize();

    vfs_t() {
        m_root = std::make_shared<vfs_dirent_t>("/", vfs_filetype_t::DIR);
    }
    ~vfs_t() {
        umount_all();
    }

    void mount(std::string_view path, std::shared_ptr<vfs_filesystem_t> file_system,
                vfs_mount_flags_t flags = vfs_mount_flags_t::DEFAULTS, const std::vector<std::string>& options = {}) {
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
    void umount(std::string_view path) {
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
    void umount_all() {
        std::unique_lock lock (m_mutex);

        m_root->m_children.clear();
        m_root->m_is_mountpoint = false;
        m_root->m_symlink_target_path = "";
        m_root->m_path_segment = "/";

        m_mountpoints.clear();
    }

    bool exists(std::string_view path) {
        try {
            std::shared_lock lock (m_mutex);
            return read_dir(path) != nullptr;
        } catch (std::exception&) {
            return false;
        }
    }
    std::size_t file_size(std::string_view path) {
        std::shared_lock lock (m_mutex);
        auto dirent = read_dir(path);
        return dirent->m_file_system.lock()->file_size(path, dirent);
    }
    vfs_fileinfo_t file_info(std::string_view path) {
        std::shared_lock lock (m_mutex);
        auto dirent = read_dir(path);
        return dirent->m_file_system.lock()->file_info(path, dirent);
    }
    vfs_filetype_t filetype(std::string_view path) {
        std::shared_lock lock (m_mutex);
        return read_dir(path)->m_filetype;
    }
    bool is_dir(std::string_view path) {
        return filetype(path) == vfs_filetype_t::DIR;
    }
    bool is_symlink(std::string_view path) {
        return filetype(path) == vfs_filetype_t::LNK;
    }
    bool is_mountpoint(std::string_view path) {
        std::shared_lock lock (m_mutex);
        return read_dir(path)->m_is_mountpoint;
    }
    vfs_mountpoint_info_t get_mount_info(std::string_view path) {
        std::shared_lock lock (m_mutex);
        if (!read_dir(path)->m_is_mountpoint) {
            throw std::runtime_error("vfs_t::get_mount_info error: " + std::string(path) + " is not a mountpoint");
        }
        return m_mountpoint_infos[path];
    }

    bool read(std::string_view path, std::uint8_t* dest, std::size_t length) {
        std::shared_ptr<vfs_dirent_t> dirent;
        {
            std::shared_lock lock (m_mutex);
            dirent = read_dir(path);
        }
        return dirent->m_file_system.lock()->read(path, dest, length, dirent);
    }
    bool read_at(std::string_view path, std::size_t offset, std::uint8_t* dest, std::size_t length) {
        std::shared_ptr<vfs_dirent_t> dirent;
        {
            std::shared_lock lock (m_mutex);
            dirent = read_dir(path);
        }
        return dirent->m_file_system.lock()->read_at(path, offset, dest, length, dirent);
    }
    void write(std::string_view path, const std::uint8_t* src, std::size_t length) {
        if (!exists(path)) touch(path);
        std::shared_ptr<vfs_dirent_t> dirent;
        {
            std::shared_lock lock (m_mutex);
            dirent = read_dir(path);
        }
        if (static_cast<std::uint16_t>(dirent->m_file_system.lock()->mount_flags()) & static_cast<std::uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
            throw std::runtime_error("vfs_t::write error: " + std::string(path) + " is read only.");
        }
        dirent->m_file_system.lock()->write(path, src, length, dirent);
    }
    void write_at(std::string_view path, std::size_t offset, const std::uint8_t* src, std::size_t length) {
        if (!exists(path)) touch(path);
        std::shared_ptr<vfs_dirent_t> dirent;
        {
            std::shared_lock lock (m_mutex);
            dirent = read_dir(path);
        }
        if (static_cast<std::uint16_t>(dirent->m_file_system.lock()->mount_flags()) & static_cast<std::uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
            throw std::runtime_error("vfs_t::write_at error: " + std::string(path) + " is read only.");
        }
        dirent->m_file_system.lock()->write_at(path, offset, src, length, dirent);
    }

    void mkdir(std::string_view path) {
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
    void touch(std::string_view path) {
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
    void symlink(std::string_view path, std::string_view target) {
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
    void rmdir(std::string_view path) {
        std::shared_ptr<vfs_dirent_t> dirent;
        {
            std::shared_lock lock (m_mutex);
            dirent = read_dir(path);
        }
        dirent->m_file_system.lock()->rmdir(path, dirent);
    }
    void rm(std::string_view path) {
        std::shared_ptr<vfs_dirent_t> dirent;
        {
            std::shared_lock lock (m_mutex);
            dirent = read_dir(path);
        }
        dirent->m_file_system.lock()->rm(path, dirent);
    }
    void hardlink(std::string_view path, std::string_view target) {
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

    static std::vector<std::string> get_path_components(std::string_view path) {
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
    static bool is_absolute(std::string_view path) {
        if (path.empty()) return false;
        return path.starts_with('/');
    }
    static std::string to_absolute(std::string_view base, std::string_view path) {
        if (!is_absolute(base)) throw std::runtime_error("vfs_t::to_absolute error: base path is not absolute");
        if (is_absolute(path)) return std::string(path);
        return std::string(base) + "/" + std::string(path);
    }
    static std::string normalize_absolute(std::string_view path) {
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
};

std::shared_ptr<vfs_t> g_vfs;

struct ramfs_t final : public vfs_filesystem_t {
private:
    std::shared_ptr<vfs_dirent_t> m_root;
    std::string m_root_path;

    std::vector<ramfs_inode_t> m_inodes;
    std::vector<std::size_t> m_available_inodes;
public:
    ramfs_t() {
        m_root = std::make_shared<vfs_dirent_t>("/", vfs_filetype_t::DIR);
        m_root->m_nid = 0;

        std::uint64_t ctime = static_cast<std::uint64_t>(std::time(nullptr));
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
    void form_dirent_tree(std::shared_ptr<vfs_dirent_t> dest) override {
        dest->m_children = m_root->m_children;
        dest->m_filetype = m_root->m_filetype;
        dest->m_nid = m_root->m_nid;
        dest->m_symlink_target_path = m_root->m_symlink_target_path;
    }
    void set_root(std::string_view path) override { m_root_path = path; }
    vfs_mount_flags_t mount_flags() const override {
        return g_vfs->get_mount_info(m_root_path).m_flags;
    }
    std::vector<std::string> mount_options() const override {
        return g_vfs->get_mount_info(m_root_path).m_options;
    }
    std::string_view get_root() const override {
        return m_root_path;
    }

    std::size_t file_size(std::string_view, std::shared_ptr<vfs_dirent_t> dirent) override {
        ramfs_inode_t& inode = m_inodes[dirent->m_nid];
        return inode.m_bytes.size();
    }
    vfs_fileinfo_t file_info(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) override {
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

    bool read(std::string_view path, std::uint8_t* dest, std::size_t length, std::shared_ptr<vfs_dirent_t> dirent) override {
        ramfs_inode_t& inode = m_inodes[dirent->m_nid];
        inode.m_atime = std::time(nullptr);
        if (dirent->m_filetype != vfs_filetype_t::REG) {
            throw std::runtime_error("ramfs_t::read error: can only read 'regular' files");
        }
        if (length > file_size(path, dirent)) {
            throw std::runtime_error("ramfs_t::read error: out of bounds.");
        }
        std::copy(inode.m_bytes.begin(), inode.m_bytes.begin() + length, dest);
        return true;
    }
    bool read_at(std::string_view path, std::size_t offset, std::uint8_t* dest, std::size_t length, std::shared_ptr<vfs_dirent_t> dirent) override {
        ramfs_inode_t& inode = m_inodes[dirent->m_nid];
        inode.m_atime = std::time(nullptr);
        if (dirent->m_filetype != vfs_filetype_t::REG) {
            throw std::runtime_error("ramfs_t::read_at error: can only read 'regular' files");
        }
        if (length + offset > file_size(path, dirent)) {
            throw std::runtime_error("ramfs_t::read_at error: out of bounds.");
        }
        std::copy(inode.m_bytes.begin() + offset, inode.m_bytes.begin() + offset + length, dest);
        return true;
    }
    void write(std::string_view, const std::uint8_t* src, std::size_t length, std::shared_ptr<vfs_dirent_t> dirent) override {
        if (static_cast<std::uint16_t>(mount_flags()) & static_cast<std::uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
            throw std::runtime_error("ramfs_t::write error: " + std::string(m_root_path) + " is read-only");
        }
        ramfs_inode_t& inode = m_inodes[dirent->m_nid];
        inode.m_atime = std::time(nullptr);
        inode.m_mtime = inode.m_atime;
        if (dirent->m_filetype != vfs_filetype_t::REG) {
            throw std::runtime_error("ramfs_t::write error: can only read 'regular' files");
        }
        inode.m_bytes.resize(length);
        std::copy(src, src + length, inode.m_bytes.begin());
    }
    void write_at(std::string_view, std::size_t offset, const std::uint8_t* src, std::size_t length, std::shared_ptr<vfs_dirent_t> dirent) override {
        if (static_cast<std::uint16_t>(mount_flags()) & static_cast<std::uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
            throw std::runtime_error("ramfs_t::write_at error: " + std::string(m_root_path) + " is read-only");
        }
        ramfs_inode_t& inode = m_inodes[dirent->m_nid];
        inode.m_atime = std::time(nullptr);
        inode.m_mtime = inode.m_atime;
        if (dirent->m_filetype != vfs_filetype_t::REG) {
            throw std::runtime_error("ramfs_t::write_at error: can only read 'regular' files");
        }
        std::size_t required_size = offset + length;
        if (required_size > inode.m_bytes.size()) {
            inode.m_bytes.resize(required_size);
        }
        std::copy(src, src + length, inode.m_bytes.begin() + offset);
    }

    void mkdir(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent_holder) override {
        if (static_cast<std::uint16_t>(mount_flags()) & static_cast<std::uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
            throw std::runtime_error("ramfs_t::mkdir error: " + std::string(m_root_path) + " is read-only");
        }
        vfs_fileinfo_t file_info = g_vfs->file_info(g_vfs->normalize_absolute(std::string(path) + "/.."));
        dirent_holder->m_nid = m_inodes.size();
        std::uint64_t ctime = static_cast<std::uint64_t>(std::time(nullptr));
        if (m_available_inodes.empty()) m_inodes.push_back(ramfs_inode_t{
            .m_ino = dirent_holder->m_nid,
            .m_ctime = ctime,
            .m_mtime = ctime,
            .m_atime = ctime,
            .m_nlink = 1,
            .m_bytes{}
        }); else {
            std::size_t nid = m_available_inodes.back();
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
    void touch(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent_holder) override {
        if (static_cast<std::uint16_t>(mount_flags()) & static_cast<std::uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
            throw std::runtime_error("ramfs_t::touch error: " + std::string(m_root_path) + " is read-only");
        }
        vfs_fileinfo_t file_info = g_vfs->file_info(g_vfs->normalize_absolute(std::string(path) + "/.."));
        dirent_holder->m_nid = m_inodes.size();
        std::uint64_t ctime = static_cast<std::uint64_t>(std::time(nullptr));
        if (m_available_inodes.empty()) m_inodes.push_back(ramfs_inode_t{
            .m_ino = dirent_holder->m_nid,
            .m_ctime = ctime,
            .m_mtime = ctime,
            .m_atime = ctime,
            .m_nlink = 1,
            .m_bytes{}
        }); else {
            std::size_t nid = m_available_inodes.back();
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
    void symlink(std::string_view path, std::string_view, std::shared_ptr<vfs_dirent_t> dirent_holder) override {
        if (static_cast<std::uint16_t>(mount_flags()) & static_cast<std::uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
            throw std::runtime_error("ramfs_t::symlink error: " + std::string(m_root_path) + " is read-only");
        }
        vfs_fileinfo_t file_info = g_vfs->file_info(g_vfs->normalize_absolute(std::string(path) + "/.."));
        dirent_holder->m_nid = m_inodes.size();
        std::uint64_t ctime = static_cast<std::uint64_t>(std::time(nullptr));
        if (m_available_inodes.empty()) m_inodes.push_back(ramfs_inode_t{
            .m_ino = dirent_holder->m_nid,
            .m_ctime = ctime,
            .m_mtime = ctime,
            .m_atime = ctime,
            .m_nlink = 1,
            .m_bytes{}
        }); else {
            std::size_t nid = m_available_inodes.back();
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
    void rmdir(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) override {
        if (static_cast<std::uint16_t>(mount_flags()) & static_cast<std::uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
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
    void rm(std::string_view path, std::shared_ptr<vfs_dirent_t> dirent) override {
        if (static_cast<std::uint16_t>(mount_flags()) & static_cast<std::uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
            throw std::runtime_error("ramfs_t::rm error: " + std::string(m_root_path) + " is read-only");
        }
        auto parent_dirent = g_vfs->read_dir(g_vfs->normalize_absolute(std::string(path) + "/.."));
        auto inode = m_inodes[dirent->m_nid];
        inode.m_nlink--;
        if (!inode.m_nlink) m_available_inodes.push_back(dirent->m_nid);
        parent_dirent->m_children.erase(std::string(dirent->m_path_segment));
    }
    void hardlink(std::string_view, std::shared_ptr<vfs_dirent_t> dirent, std::shared_ptr<vfs_dirent_t> dirent_holder) override {
        if (static_cast<std::uint16_t>(mount_flags()) & static_cast<std::uint16_t>(vfs_mount_flags_t::READ_ONLY)) {
            throw std::runtime_error("ramfs_t::hardlink error: " + std::string(m_root_path) + " is read-only");
        }
        dirent_holder->m_nid = dirent->m_nid;
        dirent_holder->m_filetype = dirent->m_filetype;
        dirent_holder->m_children = dirent->m_children;
        dirent_holder->m_is_mountpoint = dirent->m_is_mountpoint;
        auto inode = m_inodes[dirent->m_nid];
        inode.m_nlink++;
    }
};

struct vfs_file_handle_t final : public file_handle_t {
private:
    std::string m_path;
    std::ios::openmode m_mode;
    std::atomic<std::size_t> m_offset;
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

    bool open(std::string_view path, std::ios::openmode flags) override {
        if (m_path != path) {
            m_path = path;
            m_mode = flags;
            if (flags & std::ios::ate) m_offset.store(size());
            else m_offset.store(0);

            if (flags & std::ios::out && !g_vfs->exists(path)) g_vfs->touch(path);
            return is_open();
        }
        return false;
    }
    void close() override {
        if (is_open()) {
            m_path = "";
            m_mode = std::ios::ate;
        }
    }

    bool is_open() const override {
        return g_vfs->exists(m_path);
    }
    bool can_read() const override {
        return m_mode & std::ios::in;
    }
    bool can_write() const override {
        return m_mode & std::ios::out;
    }
    std::ios::openmode mode() const override {
        return m_mode;
    }
    std::string_view path() const override {
        return m_path;
    }
    std::size_t size() const override {
        return g_vfs->file_size(m_path);
    }

    std::size_t tell() const override {
        if (!is_open()) throw std::runtime_error("vfs_file_handle_t::tell error: file handle is not open");
        return m_offset.load();
    }
    std::size_t seek(std::int64_t pos, file_io_whence_t whence = file_io_whence_t::START) override {
        std::size_t pos_prev = tell(), filesize = size();
        switch (whence) {
        case file_io_whence_t::START: {
            m_offset.store(pos);
            break;
        }
        case file_io_whence_t::CUR: {
            if (pos < 0) {
                std::int64_t pos_positive = -pos;
                std::size_t pos_unsigned_safe = pos_positive;
                if (pos_prev >= pos_unsigned_safe) m_offset.fetch_sub(pos_unsigned_safe);
                else m_offset.store(0);
            } else m_offset.fetch_add(pos);
            break;
        }
        case file_io_whence_t::END: {
            if (pos < 0) {
                std::int64_t pos_positive = -pos;
                std::size_t pos_unsigned_safe = pos_positive;
                if (pos_prev >= pos_unsigned_safe) m_offset.fetch_sub(pos_unsigned_safe);
                else m_offset.store(0);
            }
            break;
        }
        default: break;
        }
        if (tell() > filesize) m_offset.store(filesize);
        return pos_prev;
    }

    bool read(std::uint8_t* dest, std::size_t length) override {
        if (!is_open()) throw std::runtime_error("vfs_file_handle_t::read error: file handle is not open");
        if (!can_read()) throw std::runtime_error("vfs_file_handle_t::read error: file handle is not open in read mode");
        return g_vfs->read_at(m_path, seek(length, file_io_whence_t::CUR), dest, length);
    }
    bool read_at(std::size_t pos, std::uint8_t* dest, std::size_t length) override {
        if (!is_open()) throw std::runtime_error("vfs_file_handle_t::read_at error: file handle is not open");
        if (!can_read()) throw std::runtime_error("vfs_file_handle_t::read_at error: file handle is not open in read mode");
        return g_vfs->read_at(m_path, pos, dest, length);
    }
    void write(const std::uint8_t* src, std::size_t length) override {
        if (!is_open()) throw std::runtime_error("vfs_file_handle_t::write error: file handle is not open");
        if (!can_write()) throw std::runtime_error("vfs_file_handle_t::write error: file handle is not open in write mode");
        g_vfs->write_at(m_path, seek(length, file_io_whence_t::CUR), src, length);
    }
    void write_at(std::size_t pos, const std::uint8_t* src, std::size_t length) override {
        if (!is_open()) throw std::runtime_error("vfs_file_handle_t::write_at error: file handle is not open");
        if (!can_write()) throw std::runtime_error("vfs_file_handle_t::write_at error: file handle is not open in write mode");
        g_vfs->write_at(m_path, pos, src, length);
    }
};

void vfs_t::initialize() {
    mount("/", std::make_shared<ramfs_t>());
    mkdir("/mnt");
}

};
