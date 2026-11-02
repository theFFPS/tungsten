/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/util/file_io.hpp>
#include <filesystem>

namespace tungsten {

std::shared_ptr<asio::io_context> g_file_io_ctx;

bool asio_file_handle_t::open(const std::string_view& path, std::ios::openmode flags) {
    if (!is_open()) {
        m_path = path;
        m_mode = flags;
        asio::file_base::flags asio_flags;
        if (flags & std::ios::out) {
            if (flags & std::ios::in) asio_flags = asio::file_base::flags::read_write;
            else asio_flags = asio::file_base::flags::write_only | asio::file_base::flags::create;
        } else {
            if (flags & std::ios::out) asio_flags = asio::file_base::flags::read_write;
            else asio_flags = asio::file_base::flags::read_only;
        }
        if (flags & std::ios::trunc) asio_flags |= asio::file_base::flags::truncate;
        if (flags & std::ios::app) asio_flags |= asio::file_base::flags::append;
        if (flags & std::ios::ate) m_offset.store(size());
        else m_offset.store(0);
        m_f = std::make_shared<asio::random_access_file>(*g_file_io_ctx, m_path, asio_flags);
        return m_f->is_open();
    }
    return false;
}
void asio_file_handle_t::close() {
    if (is_open()) {
        m_f->close();
        m_f = nullptr;
        m_path = "";
        m_mode = std::ios::ate;
    }
}

bool asio_file_handle_t::is_open() const {
    if (!m_f) return false;
    return m_f->is_open();
}
bool asio_file_handle_t::can_read() const {
    return m_mode & std::ios::in;
}
bool asio_file_handle_t::can_write() const {
    return m_mode & std::ios::out;
}
std::ios::openmode asio_file_handle_t::mode() const {
    return m_mode;
}
std::string_view asio_file_handle_t::path() const {
    return m_path;
}
size_t asio_file_handle_t::size() const {
    return std::filesystem::file_size(m_path);
}

bool asio_file_handle_t::read(uint8_t* dest, size_t length) {
    if (!is_open()) throw std::runtime_error("asio_file_handle_t::read error: file handle is not open");
    if (!can_read()) throw std::runtime_error("asio_file_handle_t::read error: file handle is not open in read mode");

    std::promise<size_t> promise;
    auto future = promise.get_future();

    m_f->async_read_some_at(tell(), asio::buffer(dest, length), [&promise](std::error_code ec, std::size_t n) {
        promise.set_value(ec ? 0 : n);
    });

    size_t pos_start = seek(future.get(), file_io_whence_t::CUR);
    return tell() - pos_start == length;
}

bool asio_file_handle_t::read_at(size_t pos, uint8_t* dest, size_t length) {
    if (!is_open()) throw std::runtime_error("asio_file_handle_t::read_at error: file handle is not open");
    if (!can_read()) throw std::runtime_error("asio_file_handle_t::read_at error: file handle is not open in read mode");

    std::promise<size_t> promise;
    auto future = promise.get_future();

    m_f->async_read_some_at(pos, asio::buffer(dest, length), [&promise](std::error_code ec, std::size_t n) {
        promise.set_value(ec ? 0 : n);
    });

    return future.get() == length;
}

void asio_file_handle_t::write(const uint8_t* src, size_t length) {
    if (!is_open()) throw std::runtime_error("asio_file_handle_t::write error: file handle is not open");
    if (!can_write()) throw std::runtime_error("asio_file_handle_t::write error: file handle is not open in write mode");

    std::promise<size_t> promise;
    auto future = promise.get_future();

    m_f->async_write_some_at(tell(), asio::buffer(src, length), [&promise](std::error_code ec, std::size_t n) {
        promise.set_value(ec ? 0 : n);
    });

    seek(future.get(), file_io_whence_t::CUR);
}

void asio_file_handle_t::write_at(size_t pos, const uint8_t* src, size_t length) {
    if (!is_open()) throw std::runtime_error("asio_file_handle_t::write_at error: file handle is not open");
    if (!can_write()) throw std::runtime_error("asio_file_handle_t::write_at error: file handle is not open in write mode");

    std::promise<size_t> promise;
    auto future = promise.get_future();

    m_f->async_write_some_at(pos, asio::buffer(src, length), [&promise](std::error_code ec, std::size_t n) {
        promise.set_value(ec ? 0 : n);
    });

    future.wait();
}

size_t asio_file_handle_t::tell() const {
    if (!is_open()) throw std::runtime_error("asio_file_handle_t::tell error: file handle is not open");
    return m_offset.load();
}
size_t asio_file_handle_t::seek(int64_t pos, file_io_whence_t whence) {
    size_t pos_prev = tell(), filesize = size();
    switch (whence) {
    case file_io_whence_t::START: {
        m_offset.store(pos);
        break;
    }
    case file_io_whence_t::CUR: {
        if (pos < 0) {
            int64_t pos_positive = -pos;
            size_t pos_unsigned_safe = pos_positive;
            if (pos_prev >= pos_unsigned_safe) m_offset.fetch_sub(pos_unsigned_safe);
            else m_offset.store(0);
        } else m_offset.fetch_add(pos);
        break;
    }
    case file_io_whence_t::END: {
        if (pos < 0) {
            int64_t pos_positive = -pos;
            size_t pos_unsigned_safe = pos_positive;
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

}
