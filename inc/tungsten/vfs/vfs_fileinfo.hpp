/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <cstdint>
#include <string>

namespace tungsten {

enum class vfs_filetype_t : uint8_t {
    UNKNOWN = 0,
    REG = 1,
    DIR = 2,
    LNK = 3
};
enum class vfs_mount_flags_t : uint16_t {
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
    uint64_t m_nid;

    vfs_filetype_t m_filetype;

    uint64_t m_ctime;
    uint64_t m_mtime;
    uint64_t m_atime;
};

}
