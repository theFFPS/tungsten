/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

module;
#include <asio.hpp>

module tungsten.io;

import std;

namespace tungsten {

std::shared_ptr<asio::io_context> g_file_io_ctx;
std::shared_ptr<asio::executor_work_guard<asio::io_context::executor_type>> g_work_guard;
std::thread g_worker_thread;

struct asio_file_handle_t final : public file_handle_t {
private:
    std::string m_path;
    std::ios::openmode m_mode;

    std::shared_ptr<asio::random_access_file> m_f = nullptr;
    std::atomic<std::size_t> m_offset;
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

    bool open(std::string_view path, std::ios::openmode flags) override {
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
    void close() override {
        if (is_open()) {
            m_f->close();
            m_f = nullptr;
            m_path = "";
            m_mode = std::ios::ate;
        }
    }

    bool is_open() const override {
        if (!m_f) return false;
        return m_f->is_open();
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
        return std::filesystem::file_size(m_path);
    }

    std::size_t tell() const override {
        if (!is_open()) throw std::runtime_error("asio_file_handle_t::tell error: file handle is not open");
        return m_offset.load();
    }
    std::size_t seek(std::int64_t pos, file_io_whence_t whence = file_io_whence_t::START) override {
        std::size_t pos_prev = tell(), filesize = size();
        switch (whence) {
        case file_io_whence_t::START: {
            if (pos < 0) m_offset.store(0);
            else m_offset.store(pos);
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
            std::int64_t target = static_cast<std::int64_t>(filesize) + pos;
            if (target < 0) m_offset.store(0);
            else m_offset.store(static_cast<std::size_t>(target));
            break;
        }
        default: break;
        }
        if (tell() > filesize) m_offset.store(filesize);
        return pos_prev;
    }

    bool read(std::uint8_t* dest, std::size_t length) override {
        if (!is_open()) throw std::runtime_error("asio_file_handle_t::read error: file handle is not open");
        if (!can_read()) throw std::runtime_error("asio_file_handle_t::read error: file handle is not open in read mode");

        std::promise<std::size_t> promise;
        auto future = promise.get_future();

        m_f->async_read_some_at(tell(), asio::buffer(dest, length), [&promise](std::error_code ec, std::size_t n) {
            promise.set_value(ec ? 0 : n);
        });

        std::size_t pos_start = seek(future.get(), file_io_whence_t::CUR);
        return tell() - pos_start == length;
    }
    bool read_at(std::size_t pos, std::uint8_t* dest, std::size_t length) override {
        if (!is_open()) throw std::runtime_error("asio_file_handle_t::read_at error: file handle is not open");
        if (!can_read()) throw std::runtime_error("asio_file_handle_t::read_at error: file handle is not open in read mode");

        std::promise<std::size_t> promise;
        auto future = promise.get_future();

        m_f->async_read_some_at(pos, asio::buffer(dest, length), [&promise](std::error_code ec, std::size_t n) {
            promise.set_value(ec ? 0 : n);
        });

        return future.get() == length;
    }
    void write(const std::uint8_t* src, std::size_t length) override {
        if (!is_open()) throw std::runtime_error("asio_file_handle_t::write error: file handle is not open");
        if (!can_write()) throw std::runtime_error("asio_file_handle_t::write error: file handle is not open in write mode");

        std::promise<std::size_t> promise;
        auto future = promise.get_future();

        m_f->async_write_some_at(tell(), asio::buffer(src, length), [&promise](std::error_code ec, std::size_t n) {
            promise.set_value(ec ? 0 : n);
        });

        seek(future.get(), file_io_whence_t::CUR);
    }
    void write_at(std::size_t pos, const std::uint8_t* src, std::size_t length) override {
        if (!is_open()) throw std::runtime_error("asio_file_handle_t::write_at error: file handle is not open");
        if (!can_write()) throw std::runtime_error("asio_file_handle_t::write_at error: file handle is not open in write mode");

        std::promise<std::size_t> promise;
        auto future = promise.get_future();

        m_f->async_write_some_at(pos, asio::buffer(src, length), [&promise](std::error_code ec, std::size_t n) {
            promise.set_value(ec ? 0 : n);
        });

        future.wait();
    }
};

std::shared_ptr<file_handle_t> make_asio_file_handle() {
    return std::make_shared<asio_file_handle_t>();
}
std::shared_ptr<file_handle_t> make_asio_file_handle(std::string_view path, std::ios::openmode mode) {
    return std::make_shared<asio_file_handle_t>(path, mode);
}

void init_asio() {
    g_file_io_ctx = std::make_shared<asio::io_context>();
    g_work_guard = std::make_shared<asio::executor_work_guard<asio::io_context::executor_type>>(asio::make_work_guard(*g_file_io_ctx));
    g_worker_thread = std::thread([]() { if (g_file_io_ctx) g_file_io_ctx->run(); });
}
void stop_asio() {
    if (g_file_io_ctx) g_file_io_ctx->stop();
    g_work_guard.reset();
    if (g_worker_thread.joinable()) g_worker_thread.join();
    g_file_io_ctx = nullptr;
}

}
