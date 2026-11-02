/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <string_view>
#include <cstdint>
#include <ios>
#include <memory>
#include <asio.hpp>

namespace tungsten {

enum class file_io_whence_t : uint8_t { START, CUR, END };

struct file_handle_t {
    virtual ~file_handle_t() = default;

    virtual bool open(std::string_view path, std::ios::openmode flags) = 0;
    virtual void close() = 0;

    virtual bool is_open() const = 0;
    virtual bool can_read() const = 0;
    virtual bool can_write() const = 0;
    virtual std::ios::openmode mode() const = 0;
    virtual std::string_view path() const = 0;
    virtual size_t size() const = 0;

    virtual size_t tell() const = 0;
    virtual size_t seek(int64_t pos, file_io_whence_t whence = file_io_whence_t::START) = 0;

    virtual bool read(uint8_t* dest, size_t length) = 0;
    virtual bool read_at(size_t pos, uint8_t* dest, size_t length) = 0;
    virtual void write(const uint8_t* src, size_t length) = 0;
    virtual void write_at(size_t pos, const uint8_t* src, size_t length) = 0;
};

struct asio_file_handle_t final : public file_handle_t {
private:
    std::string m_path;
    std::ios::openmode m_mode;

    std::shared_ptr<asio::random_access_file> m_f = nullptr;
    std::atomic<size_t> m_offset;
public:
    asio_file_handle_t() : m_mode(std::ios::ate) {
        m_offset.store(0);
    }
    asio_file_handle_t(std::string_view path, std::ios::openmode mode = std::ios::in | std::ios::out) {
        open(path, mode);
    }
    ~asio_file_handle_t() {
        close();
    }

    asio_file_handle_t(const asio_file_handle_t&) = delete;
    void operator=(const asio_file_handle_t&) = delete;

    asio_file_handle_t(asio_file_handle_t&& other) = delete;
    void operator=(asio_file_handle_t&& other) = delete;

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

extern std::shared_ptr<asio::io_context> g_file_io_ctx;

}
