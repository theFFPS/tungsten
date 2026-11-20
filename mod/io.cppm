/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

export module tungsten.io;

import std;

export namespace tungsten {

enum class file_io_whence_t : std::uint8_t { START, CUR, END };

struct file_handle_t {
    virtual ~file_handle_t() = default;

    virtual bool open(std::string_view path, std::ios::openmode flags) = 0;
    virtual void close() = 0;

    virtual bool is_open() const = 0;
    virtual bool can_read() const = 0;
    virtual bool can_write() const = 0;
    virtual std::ios::openmode mode() const = 0;
    virtual std::string_view path() const = 0;
    virtual std::size_t size() const = 0;

    virtual std::size_t tell() const = 0;
    virtual std::size_t seek(std::int64_t pos, file_io_whence_t whence = file_io_whence_t::START) = 0;

    virtual bool read(std::uint8_t* dest, std::size_t length) = 0;
    virtual bool read_at(std::size_t pos, std::uint8_t* dest, std::size_t length) = 0;
    virtual void write(const std::uint8_t* src, std::size_t length) = 0;
    virtual void write_at(std::size_t pos, const std::uint8_t* src, std::size_t length) = 0;
};

std::shared_ptr<file_handle_t> make_asio_file_handle();
std::shared_ptr<file_handle_t> make_asio_file_handle(std::string_view path, std::ios::openmode mode = std::ios::in | std::ios::out);

void init_asio();
void stop_asio();

}
